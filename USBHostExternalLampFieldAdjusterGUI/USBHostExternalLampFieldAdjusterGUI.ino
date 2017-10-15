//this is a separate sketch that loads into an arduino enigma machine simulator and adjusts the lamp brightness on the external lamp field
//this sketch does not need any other files save for the includes

#define TFTSeed
//#define TFTAdafruit

#include <SPI.h>
#include <FastTftILI9341.h>
#include <SeeedTouchScreen.h>

PDQ_ILI9341 BothTft;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate

//init TouchScreen port pins
TouchScreen ts = TouchScreen(XP, YP, XM, YM);

enum brightnessStateMachine {
  IDLE,
  NUMBERS,
  UPDATE,
};

enum mainStateMachine {
  PENUP,
  WAITFORPENDOWN,
  PENDOWN,
};

char KeyPressed;
char LightUpKey = 'A';

byte Brightness = 0;
byte TempBrightness = 0;
byte TempKeyCount = 0;

void drawScreen()
{
  BothTft.fillRectangle(0, 0, 240, 320, BLACK);

  BothTft.fillRoundRect(2, 2 + 5, 116 - 10, 117 - 10, 15, RED);
  BothTft.fillRoundRect(122 + 10, 2 + 5, 116 - 10, 117 - 10, 15, RED);
  BothTft.drawString("+", 41, 45, 4, BLACK);
  BothTft.drawString("++", 157, 45, 4, BLACK);

  BothTft.fillRoundRect(2, 122 + 5, 116 - 10, 117 - 10, 15, BLUE);
  BothTft.fillRoundRect(120 + 10 , 122 + 5, 116 - 10, 118 - 10, 15, BLUE);
  BothTft.drawString("-", 41, 165, 4, BLACK);
  BothTft.drawString("--", 157, 165, 4, BLACK);

  BothTft.fillRoundRect(2, 243 + 5, 116 - 10, 79 - 10, 15, WHITE);
  BothTft.fillRoundRect(122 + 10, 243 + 5, 116 - 10, 79 - 10, 15, WHITE);
  BothTft.drawString("A --", 15, 273, 3, BLACK);
  BothTft.drawString("++ Z", 140, 273, 3, BLACK);

  updateLightUpKey();
  updateBrightness();
}

void updateLightUpKey()
{
  BothTft.fillRectangle(111, 273, 17, 22, BLACK);
  BothTft.drawChar(LightUpKey, 109, 273, 3, WHITE);
}

void updateBrightness()
{
  BothTft.fillRectangle(112, 1, 15, 255 - Brightness, BLACK);
  BothTft.fillRoundRect(112, 1 + (255 - Brightness), 15, Brightness, 3, YELLOW);  // GREEN OR YELLOW
}

byte x = 0;
byte y = 0;

void printChar(char c)
{
  BothTft.drawChar(c, x * 8, y * 8, 1, YELLOW);
  x++;
  if (x == 29)
  {
    y++;
    x = 0;
  }
  if (y == 38)
  {
    x = 0;
    y = 0;
    drawScreen();
  }
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  //TFT initialization routine:
  BothTft.TFTinit();  //init TFT library

  drawScreen();

  Serial.print(F("\n\rer=?"));
  Serial.print(F("\n\rer>er>"));
  Serial.print(LightUpKey);
}

void loop() {
  // put your main code here, to run repeatedly:

  static mainStateMachine MainSM = PENUP;
  static byte Debounce = 0;

  static brightnessStateMachine BrightSM = IDLE;

  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  if (Serial.available())
  {
    KeyPressed = Serial.read();

    //printChar(KeyPressed);

    switch (BrightSM)
    {
      case IDLE:

        if ((KeyPressed >= '0') && (KeyPressed <= '9'))
        {
          TempBrightness = KeyPressed - '0';
          TempKeyCount = 1;

          BrightSM = NUMBERS;
        }

        break;

      case NUMBERS:

        if (((KeyPressed >= '0') && (KeyPressed <= '9')))
        {
          TempBrightness = TempBrightness * 10 + KeyPressed - '0';
          TempKeyCount++;

          if (TempKeyCount == 3)
          {
            BrightSM = UPDATE;
          }
        }
        else
        {
          BrightSM = UPDATE;
        }

        break;

      case UPDATE:

        Brightness = TempBrightness;
        updateBrightness();

        BrightSM = IDLE;

        break;
    }
  }

  switch (MainSM)
  {
    case PENUP:

      // we have some minimum pressure we consider 'valid'
      // pressure of 0 means no pressing!
      if ((p.z > __PRESURE) && (p.z < 1000))
      {
        bool changed = false;
        bool changedkey = false;

        if ((p.x > 0) && (p.x < 120) && (p.y > 0) && (p.y < 115))
        {
          //send abbreviated version of the command, simpler, faster
          //less is more
          Serial.println(F("\n\rer=+"));  // Brighter=+
          changed = true;
        }

        if ((p.x > 120) && (p.x < 240) && (p.y > 0) && (p.y < 115))
        {
          Serial.println(F("\n\rer=}")); // Brighter=}
          changed = true;
        }

        if ((p.x > 0) && (p.x < 120) && (p.y > 120) && (p.y < 240))
        {
          Serial.println(F("\n\rer=-")); // Brighter=-
          changed = true;
        }

        if ((p.x > 120) && (p.x < 240) && (p.y > 120) && (p.y < 240))
        {
          Serial.println(F("\n\rer={")); // Brighter={
          changed = true;
        }

        if ((p.x > 0) && (p.x < 120) && (p.y > 240) && (p.y < 320))
        {
          LightUpKey--;
          if (LightUpKey < 'A')
          {
            LightUpKey = 'Z';
          }
          changedkey = true;
        }

        if ((p.x > 120) && (p.x < 240) && (p.y > 240) && (p.y < 320))
        {
          LightUpKey++;
          if (LightUpKey > 'Z')
          {
            LightUpKey = 'A';
          }
          changedkey = true;
        }

        if (changed)
        {
          Serial.print(F("er>er>")); // Stecker>Stecker>
          Serial.print(LightUpKey);
          updateBrightness();
          changed = false;
        }

        if (changedkey)
        {
          Serial.println(F(""));
          Serial.print(F("er>er>")); // Stecker>Stecker>
          Serial.print(LightUpKey);
          updateLightUpKey();
          changedkey = false;
        }

        Debounce = 0;
        MainSM = WAITFORPENDOWN;
      }

      break;

    case WAITFORPENDOWN:
      {
        if (p.z < __PRESURE)
        {
          MainSM = PENDOWN;
        }
      }

    case PENDOWN:
      {
        if ((p.z > __PRESURE) && (p.z < 1000))
        {
          Debounce = 0;
          MainSM = WAITFORPENDOWN;
        }

        Debounce++;

        if (Debounce > 15)
        {
          MainSM = PENUP;
        }
        break;
      }
  }

}
