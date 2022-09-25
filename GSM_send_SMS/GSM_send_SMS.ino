#include "SIM900.h"
#include <SoftwareSerial.h>
//If not used, is better to exclude the HTTP library,
//for RAM saving.
//If your sketch reboots itself proprably you have finished,
//your memory available.
//#include "inetGSM.h"

//If you want to use the Arduino functions to manage SMS, uncomment the lines below.
#include "sms.h"
SMSGSM sms;

//To change pins for Software Serial, use the two lines in GSM.cpp.

//GSM Shield for Arduino
//www.open-electronics.org
//this code is based on the example of Arduino Labs.

//Simple sketch to send and receive SMS.

#define GSMPowerPIN 5
#define GSMResetPIN 6

boolean started=false;
char smsbuffer[160];
char inpnum[20];

void setup() 
{
  pinMode(GSMPowerPIN, OUTPUT);
  pinMode(GSMResetPIN, OUTPUT);
  
  //Serial connection.
  Serial.begin(9600);
  Serial.println("GSM Shield testing.");
  Serial.println("GSM Shield power off.");
  digitalWrite(GSMPowerPIN, HIGH);
  delay(1000);
  digitalWrite(GSMPowerPIN, LOW);
  delay(4000);
  Serial.println("GSM Shield power on.");
  digitalWrite(GSMResetPIN, HIGH);
  delay(250);
  Serial.println("\nGSM Shield reset.");
  digitalWrite(GSMResetPIN, LOW);
  delay(7000);
  //Start configuration of shield with baudrate.
  //For http uses is raccomanded to use 4800 or slower.
  Serial.println("\nGSM Shield requesting.");
  started=false;
  if (gsm.begin(2400)){
    Serial.println("\nstatus(2400)=READY");
    started=true;  
  } else {
    Serial.println("\nstatus=IDLE");
    Serial.println("GSM Shield power off.");
    digitalWrite(GSMPowerPIN, HIGH);
    delay(1000);
    digitalWrite(GSMPowerPIN, LOW);
    delay(4000);
    Serial.println("GSM Shield power on.");
    if (gsm.begin(9600)){
      Serial.println("\nstatus(9600)=READY");
      started=true;  
    }
  }
  
  if(started){
    //Enable this two lines if you want to send an SMS.
    if (sms.SendSMS("+79213300032", "Humidor ALARM! Humidity low."))
      Serial.println("\nSMS sent OK");
  }

};

void loop() 
{
  if(started){
    //Read if there are messages on SIM card and print them.
    if(gsm.readSMS(smsbuffer, 160, inpnum, 20))
    {
      Serial.println(inpnum);
      Serial.println(smsbuffer);
    }
    delay(1000);
  }
};
