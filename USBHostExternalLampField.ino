//-----------------------
// Order the lamp field board at https://oshpark.com/shared_projects/CHp3OJUL
// square leg of 6 pin header goes into Gravitech Shield Pin A5, header covers A0..A5
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

//#define USBResetPin 3
//#define USBGPXPin 7
//#define USBIntPin 2

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

#include "Adafruit_Thermal.h"

#include "SoftwareSerial.h"
#define TX_GND 4 // Ground reference  BLACK WIRE   labeled GND on printer
#define TX_PIN 5 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 6 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor
// Then see setup() function regarding serial & printer begin() calls.

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

  pinMode(MorsePIN, OUTPUT);
  LightOff();
  AllOff();

  //black into D4
  //yellow into D5
  //green into D6
  pinMode(TX_GND, OUTPUT); digitalWrite(TX_GND, LOW);
  mySerial.begin(19200);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)
  printer.setSize('L');        // Set type size, accepts 'S', 'M', 'L'

  //  pinMode(USBResetPin, OUTPUT);
  //  pinMode(USBGPXPin, INPUT);
  //  pinMode(USBIntPin, INPUT);

  //  digitalWrite(USBResetPin, 0);
  //  delay(100);
  //  digitalWrite(USBResetPin, 1);

  if (Usb.Init() == -1)
    Serial.println(F("USB Init Failed"));

  delay(100);

}

void loop()
{
  Usb.Task();

  if (Acm.isReady())
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

