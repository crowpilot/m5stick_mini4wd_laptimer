//ultrasonic sensor laptimer
//M5Stick-C + grove Ultrasonic ranger
//not include ultrasonic library
//QR code SNS share
#include <M5StickC.h>
#include<Ticker.h>
//#include "WiFi.h"

#define MAXLAP 6

#define USONIC_PIN 33

Ticker ticker;
//Ultrasonic ultrasonic(USONIC_PIN);

//const char SSID[] = "";
//const char WIFIKEY[] = "";


int coursenum = 0;
char* coursename[] = {"OVALHOME", "JCJC", "OTHER"};

volatile unsigned long laptime[MAXLAP + 1];
volatile unsigned long splittime;
volatile unsigned long average;
volatile unsigned long besttime;
volatile int lapcount;

volatile int sensordefault;
volatile int currentrange;
volatile int trigger=0;

enum Sequence {
  SEQ_OPENING,
  SEQ_COURSE,
  SEQ_LAP,
  SEQ_MENU,
  SEQ_GOAL,
  SEQ_DETAIL,
  SEQ_SHARE
};
Sequence gSequence = SEQ_OPENING;

void setzero() {
  for (int i = 0; i < MAXLAP + 1; i++) {
    laptime[i] = 0;
  }
  splittime = 0;
  average = 0;
  besttime = 0;
  lapcount = 0;
  gSequence = SEQ_OPENING;
  
  trigger=0;
  int sum=0;
  for(int i=0;i<10;i++){
    sum+=currentrange;
    delay(100);
  }
  sensordefault=sum/10;

}

void rangetask(void* arg){
  while(1){
  pinMode(USONIC_PIN, OUTPUT);
  digitalWrite(USONIC_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(USONIC_PIN, HIGH);
  delayMicroseconds(5);//5
  digitalWrite(USONIC_PIN,LOW);
  pinMode(USONIC_PIN,INPUT);
  long duration;
  duration = pulseIn(USONIC_PIN,HIGH);
  currentrange = duration/29.4/2;
  delay(3);
  }
}

void opening() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextFont(2);
  M5.Lcd.print("opening\n");
  setzero();
  /*  M5.Lcd.print("Wi-Fi\n");
    M5.Lcd.print("*CANCEL\n");
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      if (wait % 10 == 0) {
        M5.Lcd.print(".");
      }
      if (M5.BtnA.wasPressed()) {
        WiFi.disconnect();
        break;
      }
      wait++;
      if (wait >= 600) {
        M5.Lcd.print("Wi-Fi Fail");
        WiFi.disconnect();
        delay(3000);
        break;
      }
      M5.update();
    }*/
  gSequence = SEQ_COURSE;
}
//OPENING
//SELECT COURCE
void selectcourse() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("COURSE\n");
  /*  M5.Lcd.print("Wi-Fi ");
    if (WiFi.status() == WL_CONNECTED) {
      M5.Lcd.setTextColor(TFT_GREEN);
      M5.Lcd.print("OK\n");
    } else {
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.print("NG\n");
    }
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);*/
  M5.Lcd.setCursor(0,120);
  M5.Lcd.print(sensordefault);

  int totalcourse = sizeof(coursename) / sizeof(*coursename);
  while (!M5.BtnA.wasPressed()) {
    if (M5.BtnB.wasPressed()) {
      coursenum++;
      if (coursenum >= totalcourse) {
        coursenum = 0;
      }
    }

    for (int i = 0; i < totalcourse; i++) {
      M5.Lcd.setCursor(0, 20 + 16 * i);
      M5.Lcd.printf("%c%-9s\n", i == coursenum ? '*' : ' ', coursename[i]);
    }
    M5.update();
  }
  gSequence = SEQ_LAP;
}

void count() {
  splittime++;
  laptime[lapcount]++;
  //  laptime++;
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("%02lu:%02lu.%02lu\n", (splittime / 6000) % 60, (splittime / 100) % 60, splittime % 100);
  M5.Lcd.setCursor(0, 40 + lapcount * 16);
  M5.Lcd.printf("%d %02lu:%02lu.%02lu\n", lapcount + 1, (laptime[lapcount] / 6000) % 60, (laptime[lapcount] / 100) % 60, laptime[lapcount] % 100);

  // M5.Lcd.print(splittime%100);
}


void lap() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("LAP");
  ticker.attach_ms(10, count);
  while (1) {
    if (M5.BtnA.wasPressed()) {
      gSequence = SEQ_MENU;
      ticker.detach();
      break;
    }
    //laptime
    if (sensordefault-currentrange>1&&trigger==0) {
      // laptime[lapcount] = splittime-laptime[lapcount-1];
      lapcount++;
      trigger=1;
      if (lapcount >= MAXLAP) {
        gSequence = SEQ_GOAL;
        ticker.detach();
        break;
      }
      laptime[lapcount] = 0;
    }else if(sensordefault-currentrange<=1){
      delay(100);
      trigger=0;
    }
    //  M5.Lcd.setCursor(0,20);
    //M5.Lcd.printf("%02lu:%02lu.%02lu",(splittime/6000)%60,(splittime/100)%60,splittime%100);
    M5.update();
    delay(1);
  }
  //calc average
}
//MENU
void menu() {
  int menunum = 0;
  char* menu[] = {"RESUM", "EXIT"};
  int totalmenu = sizeof(menu) / sizeof(*menu);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("MENU");
  while (1) {
    //    M5.Lcd.print("MENU");
    if (M5.BtnB.wasPressed()) {
      menunum++;
      if (menunum >= totalmenu) {
        menunum = 0;
      }
    }
    if (M5.BtnA.wasPressed()) {
      if (menunum == 0) {
        //RESUM
        M5.Lcd.fillScreen(TFT_BLACK);
        for (int i = 0; i < lapcount; i++) {
          M5.Lcd.setCursor(0, 40 + i * 16);
          M5.Lcd.printf("%d %02lu:%02lu.%02lu\n", i + 1,
                        (laptime[i] / 6000) % 60,
                        (laptime[i] / 100) % 60,
                        laptime[i] % 100);
        }
        gSequence = SEQ_LAP;

        break;
      } else if (menunum == 1) {
        //EXIT
        setzero();
        gSequence = SEQ_OPENING;
        break;
      }
    }
    M5.Lcd.setCursor(0, 20);
    for (int i = 0; i < totalmenu; i++) {
      M5.Lcd.printf("%c%-9s\n", i == menunum ? '*' : ' ', menu[i]);
    }
    M5.update();
  }
}
//MENU
//GOAL
void goal() {
  int menunum = 0;
  char* menu[] = {"SHARE", "DETAIL", "EXIT"};
  int totalmenu = sizeof(menu) / sizeof(*menu);

  besttime = laptime[0];
  for (int i = 0; i < MAXLAP; i++) {
    if (laptime[i] < besttime) {
      besttime = laptime[i];
    }
    average += laptime[i];
  }
  average = average / MAXLAP;

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("FINISH!!\n");
  M5.Lcd.printf("T %02lu:%02lu.%02lu\n", (splittime / 6000) % 60,
                (splittime / 100) % 60,
                splittime % 100);
  M5.Lcd.printf("B %02lu:%02lu.%02lu\n",
                (besttime / 6000) % 60,
                (besttime / 100) % 60,
                besttime % 100);
  M5.Lcd.printf("A %02lu:%02lu.%02lu\n",
                (average / 6000) % 60,
                (average / 100) % 60,
                average % 100);
  while (1) {
    //    M5.Lcd.print("MENU");
    if (M5.BtnB.wasPressed()) {
      menunum++;
      if (menunum >= totalmenu) {
        menunum = 0;
      }
    }
    if (M5.BtnA.wasPressed()) {
      if (menunum == 0) {
        //SHARE
        gSequence = SEQ_SHARE;
        break;
      } else if (menunum == 1) {
        //DETAIL
        gSequence = SEQ_DETAIL;
        break;
      }
      else if (menunum == 2) {
        //EXIT
        setzero();
        gSequence = SEQ_OPENING;
        break;
      }
    }
    M5.Lcd.setCursor(0, 100);
    for (int i = 0; i < totalmenu; i++) {
      M5.Lcd.printf("%c%-9s\n", i == menunum ? '*' : ' ', menu[i]);
    }
    M5.update();
  }
}

void detail() {
  int menunum = 0;
  char* menu[] = {"BACK", "EXIT"};
  int totalmenu = sizeof(menu) / sizeof(*menu);

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("DETAIL\n");
  M5.Lcd.printf("T %02lu:%02lu.%02lu\n", (splittime / 6000) % 60,
                (splittime / 100) % 60,
                splittime % 100);
  for (int i = 0; i < MAXLAP; i++) {
    M5.Lcd.setCursor(0, 32 + i * 16);
    M5.Lcd.printf("%d %02lu:%02lu.%02lu\n",
                  i + 1,
                  (laptime[i] / 6000) % 60,
                  (laptime[i] / 100) % 60,
                  laptime[i] % 100);

  }

  while (1) {
    //    M5.Lcd.print("MENU");
    if (M5.BtnB.wasPressed()) {
      menunum++;
      if (menunum >= totalmenu) {
        menunum = 0;
      }
    }
    if (M5.BtnA.wasPressed()) {
      if (menunum == 0) {
        //BACK
        gSequence = SEQ_GOAL;

        break;
      } else if (menunum == 1) {
        //EXIT
        setzero();
        gSequence = SEQ_OPENING;
        break;
      }
    }
    M5.Lcd.setCursor(0, 160 - totalmenu * 16);
    for (int i = 0; i < totalmenu; i++) {
      M5.Lcd.printf("%c%-9s\n", i == menunum ? '*' : ' ', menu[i]);
    }
    M5.update();
  }

}
//SHARE
void share() {
  int menunum = 0;
  char* menu[] = {"TWITTER", "MASTODON", "CANCEL"};
  int totalmenu = sizeof(menu) / sizeof(*menu);

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("SHARE\n");
  /*  M5.Lcd.print("Wi-Fi ");
    if (WiFi.status() == WL_CONNECTED) {
      M5.Lcd.setTextColor(TFT_GREEN);
      M5.Lcd.print("OK\n");
    } else {
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.print("NG\n");
      menunum=1;
    }
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);*/

  while (1) {
    if (M5.BtnB.wasPressed()) {
      menunum++;
      if (menunum >= totalmenu) {
        menunum = 0;
      }
    }
    if (M5.BtnA.wasPressed()) {
      M5.update();
      if (menunum == 0) {
        //TWITTER
        M5.Axp.ScreenBreath(9);
        char qr[200];
        //("T %02lu:%02lu.%02lu\n", (splittime / 6000) % 60,
        //, (splittime / 100) % 60,
        //splittime % 100);
        sprintf(qr, "https://twitter.com/intent/tweet?text=T:%02lu:%02lu.%02lu%%0aB:%02lu:%02lu.%02lu%%0aA:%02lu:%02lu.%02lu%%0a%%23CROWCUP%%5f%s",
                (splittime / 6000) % 60, (splittime / 100) % 60, splittime % 100,
                (besttime / 6000) % 60, (besttime / 100) % 60, besttime % 100,
                (average / 6000) % 60, (average / 100) % 60, average % 100,
                coursename[coursenum]
               );
        M5.Lcd.qrcode(qr, 0, 45, 80, 5);
        while (!M5.BtnA.wasPressed()) {

          M5.update();
        }
        M5.Axp.ScreenBreath(15);
        break;
      } else if (menunum == 1) {
        //MASTODN
        M5.Axp.ScreenBreath(9);
        char qr[200];
        sprintf(qr, "https://mastportal.info/intent?text=T:%02lu:%02lu.%02lu%%0aB:%02lu:%02lu.%02lu%%0aA:%02lu:%02lu.%02lu%%0a%%23CROWCUP%%5f%s",
                (splittime / 6000) % 60, (splittime / 100) % 60, splittime % 100,
                (besttime / 6000) % 60, (besttime / 100) % 60, besttime % 100,
                (average / 6000) % 60, (average / 100) % 60, average % 100,
                coursename[coursenum]
               );
        M5.Lcd.qrcode(qr, 0, 45, 80, 5);
        while (!M5.BtnA.wasPressed()) {
          M5.update();
        }
        M5.Axp.ScreenBreath(15);
        break;
      }
      /*else if(menunum==1){
          WiFi.disconnect();
          WiFi.begin(SSID,WIFIKEY);
          M5.Lcd.setCursor(0,128);
          M5.Lcd.print("CONNECTING\n");
          int wait=0;
          while(WiFi.status()!=WL_CONNECTED){
            M5.update();
            delay(100);
            if(wait%100==0){
             M5.Lcd.print(".");
            }
            wait++;
            if(wait>600||M5.BtnA.wasPressed()){
              M5.Lcd.setCursor(0,128);
              M5.Lcd.print("Wi-Fi FAIL  \n");
              break;
            }
          }
        }*/
      else if (menunum == 2) {

        gSequence = SEQ_GOAL;
        break;
      }
    }
    for (int i = 0; i < totalmenu; i++) {
      M5.Lcd.setCursor(0, 20+16*i);
      M5.Lcd.printf("%c%-9s", i == menunum ? '*' : ' ', menu[i]);
    }
    M5.update();
  }

}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  xTaskCreatePinnedToCore(rangetask,"rangetask",4096,NULL,1,NULL,1);
  //  WiFi.begin(SSID, WIFIKEY);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (gSequence) {
    case SEQ_OPENING:
      opening();
      break;
    case SEQ_COURSE:
      selectcourse();
      M5.Lcd.fillScreen(TFT_BLACK);
      break;
    case SEQ_LAP:
      lap();
      break;
    case SEQ_MENU:
      menu();
      break;
    case SEQ_GOAL:
      goal();
      break;
    case SEQ_DETAIL:
      detail();
      break;
    case SEQ_SHARE:
      share();
      break;
  }
  M5.update();
}
