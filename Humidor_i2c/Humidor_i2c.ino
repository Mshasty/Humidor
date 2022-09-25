/* Sketch for 20 character 2 - 4 line I2C Display
   Backpack Interface labelled "YwRobot Arduino LCM1602 IIC V1" */

/*-----( Импорт требуемых библиотек )-----*/
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <DHT22.h>
//#include <SIM900.h>
//#include <SoftwareSerial.h>
//#include "sms.h"
//SMSGSM sms;

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// кол-во строк дисплея
#define LCDstrNum 4
// нижний датчик
#define DHT22_1_PIN 7
// верхний датчик
#define DHT22_2_PIN 4

// Управление вентиллятором
#define FanPIN 11
// Управление увлажнителем
#define HumPIN 12
#define LedPIN 13

// до 18.10.13 было #define HumHMax 72
#define HumHMax 75
// до 18.10.13 было #define HumLMax 79
// до 30.01.14 было #define HumLMax 82
#define HumLMax 85
// до 18.10.13 было #define HumHMin 62
#define HumHMin 65
// до 18.10.13 было #define HumLMin 67
#define HumLMin 70

// диапазон рабочих температур (пока не задействован)
#define TmprMax 30
#define TmprMin 15

// Таймаут игнорирования аварий для SMS
#define TimeOutGSM 1800

// Количество пропускаемых аварийных событий до очередного BEEP'а
// Для превышения влажности (один цикл не менее двух DHTDelay
#define AlarmHigh 48
// Для недостаточной влажности
#define AlarmLow 32
// Количество пропускаемых аварий до очередного SMS
#define AlarmGSM 25
// задержка для датчика DHT22 в ms
#define DHTDelay 2000

// Вывод лога в com-порт
//#define SerialDebug true
#define SerialDebug false

float TempNow;
float HumL, HumH, HumNow;
unsigned int TimerCount;
unsigned int TimerMax;
unsigned int TimerGSM;
boolean NoMuteGSM;

void setup() {
  NoMuteGSM = false;
  /*
   Humidor  climatech
    www.freeduino.ru
      sponsored by
    Artem V.Erenburg
  */
  lcd.begin(20,LCDstrNum);         // initialize the lcd for 20 chars 4 lines, turn on backlight
  lcd.setCursor(1, 0);
  lcd.print("Humidor  climatech");
  delay(2000);
  lcd.setCursor(2, 1);
  lcd.print("www.freeduino.ru");
  if (LCDstrNum >= 3) {
    if (LCDstrNum >= 4) {
      delay(1500);
      lcd.setCursor(4, 2);
      lcd.print("sponsored by");
      delay(2500);
      lcd.setCursor(2, 3);
      lcd.print("Artem V.Erenburg");
    } else {
      delay(2000);
      lcd.setCursor(2, 2);
      lcd.print("artem@gpb.spb.ru");
    }
  }
  LCDalarm(2);
  Serial.begin(9600);
  if (SerialDebug) Serial.println("Start working.");
  pinMode(LedPIN, OUTPUT);     
  pinMode(FanPIN, OUTPUT);     
  pinMode(HumPIN, OUTPUT);     
  TimerCount = 0;
  TimerMax = 32767;
  TimerGSM = AlarmGSM;
  delay(3000);
  // clear the screen
  lcd.clear();
  if (LCDstrNum >= 4) {
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    delay(250);
    lcd.print(TmprMin);
    lcd.print("-");
    lcd.print(TmprMax);    
    lcd.print(char(223));
    lcd.print("C");    
    delay(1500);
    lcd.setCursor(1, 1);
    lcd.print("Humidity intervals");
    delay(500);
  }
  lcd.setCursor(0, LCDstrNum-2);
  lcd.print("High sensor: ");
  lcd.print(HumHMin);
  lcd.print("-");
  lcd.print(HumHMax);    
  lcd.print(" %");    
  lcd.setCursor(0, LCDstrNum-1);
  lcd.print("Low sensor:  ");
  lcd.print(HumLMin);
  lcd.print("-");
  lcd.print(HumLMax);    
  lcd.print(" %");    
  delay(500);
  // clear the screen
  LCDalarm(2);
  lcd.clear();
  delay(1000);
}

void loop() {
  if (!NoMuteGSM) {
    if (TimeOutGSM < millis()/1000) {
      NoMuteGSM = true;
      lcd.setCursor(0, 2);
      lcd.print("                    ");
    } else {
      if (LCDstrNum >= 3) {
        lcd.setCursor(15, 2);
        lcd.print("     ");
        lcd.setCursor(0, 2);
        lcd.print("GSM mute-time: ");
        lcd.print(TimeOutGSM - millis()/1000);
      }
    }
  }
  // Опрос верхнего датчика
  HumNow = getHumidity(DHT22_2_PIN);
  if (HumNow > 0) {
    HumH = HumNow;
    lcd.setCursor(0, 0);
    lcd.print("SensorH: ");
    lcd.print(HumH);
    lcd.print("% ");    
    lcd.print((int)TempNow);
    lcd.print(char(223));
    lcd.print("C");    
  }
//  delay(DHTDelay);
  // Опрос нижнего датчика
  HumNow = getHumidity(DHT22_1_PIN);
  if (HumNow > 0) {
    HumL = HumNow;
    lcd.setCursor(0, 1);
    lcd.print("SensorL: ");
    lcd.print(HumL);
    lcd.print("% ");    
    lcd.print((int)TempNow);
    lcd.print(char(223));
    lcd.print("C");    
  }
//  delay(DHTDelay);
  if (LCDstrNum >= 4) {
    lcd.setCursor(0, 3);
    lcd.print("                    ");
  }
  digitalWrite(LedPIN, HIGH);
  if (HumL < HumLMin){
    // Нижний датчик меньше минимума
    if (SerialDebug) Serial.print("SensorL <LOW>:  ");
    if (SerialDebug) Serial.println(HumL);
    if (HumH > (HumHMax-3)){
    // не фаним при превышении
      SetFan(LOW);
      SetHum(HIGH);
      TimerCount = 0;
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("   Humidity LOW   ");
      }
    } else if (HumH <= (HumHMin + 3)){
      SetFan(HIGH);
      SetHum(HIGH);
      if (SerialDebug) Serial.print("Alarm: Humidity LOW! ");
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("Alarm: Humidity LOW!");
      }
      TimerMax = AlarmLow;
      TimerCount++;
      if (SerialDebug) Serial.println(TimerCount);
    } else {
      SetFan(HIGH);
      SetHum(HIGH);
      TimerCount = 0;
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("   Humidity LOW   ");
      }
    }
  } else if (HumL > HumLMax){
    // Нижний датчик больше максимума
    if (SerialDebug) Serial.print("SensorL <HIGH>: ");
    if (SerialDebug) Serial.println(HumL);
    SetHum(LOW);
    if (HumH <= HumHMax){
      // выравниваем влажность
      SetFan(HIGH);
      TimerCount = 0;
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("   Humidity HIGH  ");
      }
    } else {
      SetFan(LOW);
      // Сигнализируем по таймеру
      TimerMax = AlarmHigh;
      TimerCount++;
      lcd.backlight();
      if (SerialDebug) Serial.print("Alarm: Humidity HIGH! ");
      if (SerialDebug) Serial.println(TimerCount);
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("Alarm: Humidity HIGH");
      }
    }
  } else {
    // нижний датчик в норме
    TimerCount = 0;
    if (HumH > HumHMax){
      // Верхний датчик больше максимума
      if (SerialDebug) Serial.print("SensorH <HIGH>: ");
      if (SerialDebug) Serial.println(HumH);
      SetHum(LOW);
      SetFan(HIGH);
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.backlight();
        lcd.print("   Humidity HIGH  ");
      }
    } else if (HumH < HumHMin){
      // Верхний датчик меньше минимума
      if (SerialDebug) Serial.print("SensorH <LOW>:  ");
      if (SerialDebug) Serial.println(HumH);
      SetFan(HIGH);
      if (HumL < (HumLMax - 2)){
        SetHum(HIGH);
      } else {
        SetHum(LOW);
      }
      lcd.backlight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("   Humidity LOW   ");
      }
    } else { // климат в норме
      digitalWrite(LedPIN, LOW);
      SetFan(LOW);
      SetHum(LOW);
      lcd.noBacklight();
      if (LCDstrNum >= 4) {
        lcd.setCursor(0, 3);
        lcd.print("     Climat OK.     ");
      }
    }
  }
  if (TimerMax < TimerCount){
    TimerCount = 0;
    if (SerialDebug) Serial.println("***<+> BEEP ALARM!!! <+>***");
    LCDalarm(7);
    // не пора ли отправить SMS ?
    if (NoMuteGSM) {
      if (TimerGSM >= AlarmGSM){
        if (SerialDebug) Serial.println("---------------------------------");
        if (SerialDebug) Serial.println("| *** Sent Alarm-SMS to GSM *** |");
        if (SerialDebug) Serial.println("---------------------------------");
        TimerGSM = 0;
        if (LCDstrNum >= 3) {
          lcd.setCursor(0, 2);
          lcd.print("   Send Alarm-SMS   ");
          delay(7000);
          lcd.setCursor(0, 2);
          lcd.print("                    ");
        }
        
      } else TimerGSM++;
    }
  }
}

void SetFan(boolean FanSW) {
  digitalWrite(FanPIN, FanSW);
}

void SetHum(boolean HumSW) {
  digitalWrite(HumPIN, HumSW);
}

float getHumidity(int SensorNum) {
  DHT22 sensorDHT22(SensorNum);
  DHT22_ERROR_t errorCode;
  
  // The sensor can only be read from every 1-2s, and requires a minimum
  // 2s warm-up after power-on.
  delay(DHTDelay);
  
  if (SerialDebug) Serial.print("Requesting data on pin ");
  if (SerialDebug) Serial.print(SensorNum);
  if (SerialDebug) Serial.print("...");
  errorCode = sensorDHT22.readData();
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      if (SerialDebug) Serial.print("Got Data ");
      if (SerialDebug) Serial.print(sensorDHT22.getTemperatureC());
      if (SerialDebug) Serial.print("C ");
      if (SerialDebug) Serial.print(sensorDHT22.getHumidity());
      if (SerialDebug) Serial.println("%");
/*      // Alternately, with integer formatting which is clumsier but more compact to store and
// can be compared reliably for equality:
//
      char buf[128];
      sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
                   sensorDHT22.getTemperatureCInt()/10, abs(sensorDHT22.getTemperatureCInt()%10),
                   sensorDHT22.getHumidityInt()/10, sensorDHT22.getHumidityInt()%10);
      if (SerialDebug) Serial.println(buf); */
      TempNow = sensorDHT22.getTemperatureC();
      return(sensorDHT22.getHumidity());
      break;
    case DHT_ERROR_CHECKSUM:
      if (SerialDebug) Serial.print("check sum error ");
      if (SerialDebug) Serial.print(sensorDHT22.getTemperatureC());
      if (SerialDebug) Serial.print("C ");
      if (SerialDebug) Serial.print(sensorDHT22.getHumidity());
      if (SerialDebug) Serial.println("%");
      return(0);
      break;
    case DHT_BUS_HUNG:
      if (SerialDebug) Serial.println("BUS Hung ");
      return(0);
      break;
    case DHT_ERROR_NOT_PRESENT:
      if (SerialDebug) Serial.println("Not Present ");
      return(0);
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      if (SerialDebug) Serial.println("ACK time out ");
      return(0);
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      if (SerialDebug) Serial.println("Sync Timeout ");
      return(0);
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      if (SerialDebug) Serial.println("Data Timeout ");
      return(0);
      break;
    case DHT_ERROR_TOOQUICK:
      if (SerialDebug) Serial.println("Polled to quick ");
      return(0);
      break;
  }
}

void LCDalarm(int acnt) { // мигаем подсветкой дисплея acnt раз
  lcd.backlight(); // finish with backlight on  
  for(int ia = 0; ia < acnt; ia++) {
    lcd.noBacklight();
    delay(250);
    lcd.backlight();
    delay(250);
  }
}  

