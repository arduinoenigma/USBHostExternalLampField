//-----------------------
// Order the lamp field board at https://oshpark.com/shared_projects/CHp3OJUL
//
// Order a Gravitech USBHOST add-on for Arduino Nano http://www.gravitech.us/usadforarna.html
// using solder, bridge all three pads, VIN, GPX, INT 
//
// Order an Arduino Nano http://www.gravitech.us/arna30wiatp.html
//
// Download and install https://github.com/felis/USB_Host_Shield_2.0
//
// Locate the following line in Libraries\Documents\Arduino\libraries\USB_Host_Shield_2.0-master\UsbCore.h
//
// typedef MAX3421e<P10, P9> MAX3421E;
//
// Change to
//
// typedef MAX3421e<P8, P2> MAX3421E;     // SS @ D8 (pin11)  INT @ D2 (pin5)
//

//-----------------------
#define USBDevBaud 9600

#define USBResetPin 3
#define USBGPXPin 7
#define USBIntPin 2

#include <cdcacm.h>
#include "USBInit.h"

//----------------------

#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <avr/eeprom.h>

#define MorsePIN 10
#define PINON 0
#define PINOFF 1

//----------------------

void LightOn()
{
  digitalWrite(MorsePIN, PINON);
}

void LightOff()
{
  digitalWrite(MorsePIN, PINOFF);
}

//-----------------------

void setup()
{
  Serial.begin(9600);

  pinMode(USBResetPin, OUTPUT);
  pinMode(USBGPXPin, INPUT);
  pinMode(USBIntPin, INPUT);

  pinMode(MorsePIN, OUTPUT);
  LightOff();
  AllOff();

  digitalWrite(USBResetPin, 0);
  delay(100);
  digitalWrite(USBResetPin, 1);

  if (Usb.Init() == -1)
    Serial.println(F("USB Init Failed"));

  delay(100);

}

void loop()
{
  Usb.Task();

  if ( Acm.isReady())
  {
    uint8_t rcode;

    // buffer size must be greater or equal to max.packet size
    // it it set to 64 (largest possible max.packet size) here
    // can be tuned down for particular endpoint
    uint8_t  buf[64];
    uint16_t rcvd = 64;
    rcode = Acm.RcvData(&rcvd, buf);

    if ( rcvd )
    {
      for (uint16_t i = 0; i < rcvd; i++ ) {
        Analyze((char)buf[i]);
      }

      //to send:
      uint8_t data = '!';
      //rcode = Acm.SndData(1, &data);
    }

  }

  // if there is anything queued to send out, send it, call often
  SendMorse();
  MininumLightTime();

}


