//
// Пример использования датчика влажности и температуры DHT11
// 

#include <LiquidCrystal.h>
#include <dht11.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
dht11 sensor;

#define DHT11PIN 8
// нижний датчик
#define DHT11LPIN 2
// верхний датчик
#define DHT11HPIN 1
// Управление вентиллятором
#define FanPIN 6
// Управление увлажнителем
#define HumPIN 7
#define LedPIN 13

#define HumLMin 58
#define HumLMax 67
#define HumHMin 55
#define HumHMax 64
// Количество событий до аварии
#define AlarmHigh 3
#define AlarmLow 2
// Количество пропускаемых аварий до очередного SMS
#define AlarmGSM 4
// задержка для датчика DHT11
// #define DHTDelay 2000
#define DHTDelay 200 // уменьшена для моделирования

float TempNow;
int HumL, HumH;
unsigned int TimerCount;
unsigned int TimerMax;
unsigned int TimerGSM;

void setup() {
  lcd.begin(20, 2);
  lcd.clear();
  Serial.begin(9600);
  Serial.println("Start working.");
  pinMode(LedPIN, OUTPUT);     
  pinMode(FanPIN, OUTPUT);     
  pinMode(HumPIN, OUTPUT);     
  TimerCount = 0;
  TimerMax = 30000;
  TimerGSM = AlarmGSM;
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Hum L: ");
  HumL = getHumidity(DHT11LPIN);
  lcd.print(HumL);
  lcd.setCursor(0, 1);
  lcd.print("Tmp L: ");
  lcd.print((int)TempNow);
  delay(DHTDelay);
  lcd.setCursor(9, 0);
  lcd.print(", H: ");
  HumH = getHumidity(DHT11HPIN);
  lcd.print(HumH);
  lcd.setCursor(9, 1);
  lcd.print(", H: ");
  lcd.print((int)TempNow);
  delay(DHTDelay);
  digitalWrite(LedPIN, LOW);
  if (HumL < HumLMin){
    // Нижний датчик меньше минимума
    Serial.print("SensorL <LOW>:  ");
    Serial.println(HumL);
    if (HumH > (HumHMax-2)){
    // не фаним при превышении
      SetFan(LOW);
      SetHum(HIGH);
      TimerCount = 0;
    } else if (HumH <= (HumHMin + 2)){
      SetFan(HIGH);
      SetHum(HIGH);
      Serial.print("Alarm: Humidity LOW! ");
      TimerMax = AlarmLow;
      TimerCount++;
      Serial.println(TimerCount);
    } else {
      SetFan(HIGH);
      SetHum(HIGH);
      TimerCount = 0;
    }
  } else if (HumL > HumLMax){
    // Нижний датчик больше максимума
    Serial.print("SensorL <HIGH>: ");
    Serial.println(HumL);
    SetHum(LOW);
    if (HumH <= HumHMax){
      // выравниваем влажность
      SetFan(HIGH);
      TimerCount = 0;
    } else {
      SetFan(LOW);
      // Сигнализируем по таймеру
      TimerMax = AlarmHigh;
      TimerCount++;
      Serial.print("Alarm: Humidity HIGH! ");
      Serial.println(TimerCount);
    }
  } else {
    // нижний датчик в норме
    TimerCount = 0;
    if (HumH > HumHMax){
      // Верхний датчик больше максимума
      Serial.print("SensorH <HIGH>: ");
      Serial.println(HumH);
      SetHum(LOW);
      SetFan(HIGH);
    } else if (HumH < HumHMin){
      // Верхний датчик меньше минимума
      Serial.print("SensorH <LOW>:  ");
      Serial.println(HumH);
      SetFan(HIGH);
      if (HumL < (HumLMax - 2)){
        SetHum(HIGH);
      } else {
        SetHum(LOW);
      }
    } else {
      digitalWrite(LedPIN, HIGH);
      SetFan(LOW);
      SetHum(LOW);
    }
  }
  if (TimerMax < TimerCount){
    TimerCount = 0;
    Serial.println("***<+> BEEP ALARM!!! <+>***");
    if (TimerGSM >= AlarmGSM){
      Serial.println("---------------------------------");
      Serial.println("| *** Sent Alarm-SMS to GSM *** |");
      Serial.println("---------------------------------");
      TimerGSM = 0;
    } else TimerGSM++;
  }
}

int getHumidity(int DHTpin) {
  TempNow = 30 - DHTpin;
  return(analogRead(DHTpin)/10);    // read the value from the sensor  
}

int getHumidityTest(byte DHTpin) {
  switch (sensor.read(DHTpin)) {
    case DHTLIB_OK: 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		lcd.setCursor(0, 0);
                lcd.print("Checksum error");
                delay(2000);
                return(0);
    case DHTLIB_ERROR_TIMEOUT: 
		lcd.setCursor(0, 0);
		lcd.print("Time out error"); 
                delay(2000);
                return(0);
    default: 
		lcd.setCursor(0, 0);
		lcd.print("Unknown error"); 
                delay(2000);
                return(0);
  }
  TempNow = sensor.temperature;
  return(sensor.humidity);
}

void SetFan(boolean FanSW) {
  digitalWrite(FanPIN, FanSW);
}

void SetHum(boolean HumSW) {
  digitalWrite(HumPIN, HumSW);
}
