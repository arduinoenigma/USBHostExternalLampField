// silence between symbols: dot
// silence between letters: dash

byte WPM = 2;             // target WPM
unsigned long dot = 0;    // dot lenght in microseconds
unsigned long dash = 0;   // dash lenght in microseconds

#define MorseBufferSize 400
char MorseBuffer[MorseBufferSize];
int MorseBufferRead = 0;
int MorseBufferWrite = 0;

// binary encoding of morse table:
// BXXXZYYYW where XXX = binary lenght of character and ZYYYW - morse code where W = first symbol, Z = last
// in symbols, 0 = dot, 1 = dash

const unsigned char MorseCode[] PROGMEM =
{
  B01011110,  //  0 - A (2) .-
  B10010001,  //  1 - B (4) -...
  B10010101,  //  2 - C (4) -.-.
  B01111001,  //  3 - D (3) -..
  B00111110,  //  4 - E (1) .
  B10010100,  //  5 - F (4) ..-.
  B01111011,  //  6 - G (3) --.
  B10010000,  //  7 - H (4) ....
  B01011100,  //  8 - I (2) ..
  B10011110,  //  9 - J (4) .---
  B01111101,  // 10 - K (3) -.-
  B10010010,  // 11 - L (4) .-..
  B01011111,  // 12 - M (2) --
  B01011101,  // 13 - N (2) -.
  B01111111,  // 14 - O (3) ---
  B10010110,  // 15 - P (4) .--.
  B10011011,  // 16 - Q (4) --.-
  B01111010,  // 17 - R (3) .-.
  B01111000,  // 18 - S (3) ...
  B00111111,  // 19 - T (1) -
  B01111100,  // 20 - U (3) ..-
  B10011000,  // 21 - V (4) ...-
  B01111110,  // 22 - W (3) .--
  B10011001,  // 23 - X (4) -..-
  B10011101,  // 24 - Y (4) -.--
  B10010011,  // 25 - Z (4) --..
  B10111111,  // 26 - 0 (5) -----
  B10111110,  // 27 - 1 (5) .----
  B10111100,  // 28 - 2 (5) ..---
  B10111000,  // 29 - 3 (5) ...--
  B10110000,  // 30 - 4 (5) ....-
  B10100000,  // 31 - 5 (5) .....
  B10100001,  // 32 - 6 (5) -....
  B10100011,  // 33 - 7 (5) --...
  B10100111,  // 34 - 8 (5) ---..
  B10101111   // 35 - 9 (5) ----.
};

// do the WPM to dot(ms) and dash(ms) conversion
void InitMorse()
{
  dot = (1200 / WPM) * (unsigned long)1000;
  dash = (dot * 3);
}

int IncBufferPtr(int oldPtr)
{
  int newv = 0;

  if ((oldPtr + 1) < MorseBufferSize)
  {
    newv = oldPtr + 1;
  }

  return newv;
}

bool BufferFull()
{
  return (IncBufferPtr(MorseBufferWrite) == MorseBufferRead);
}

bool BufferEmpty()
{
  return (MorseBufferWrite == MorseBufferRead);
}

void WriteToBuffer(char c)
{
  if (!BufferFull())
  {
    MorseBuffer[MorseBufferWrite] = c;
    MorseBufferWrite = IncBufferPtr(MorseBufferWrite);
  }
}

char ReadFromBuffer()
{
  char c = 0;

  if (!BufferEmpty())
  {
    c = MorseBuffer[MorseBufferRead];
    MorseBufferRead = IncBufferPtr(MorseBufferRead);
  }

  return c;
}

// add char c to morse code queue
void QueueMorse(char c)
{
  if (dot == 0)
  {
    InitMorse();
  }

  WriteToBuffer(c);
}


byte GetMorseIndex(char c)
{
  // converts ASCII codes into morse code table indexes
  // returns 99 if the character is not in morse table, else returns lookup table index of 0..35

  // 48.. 57 0..9 -> 26..35
  // 65.. 90 A..Z ->  0..25
  // 97..122 a..z ->  0..25

  byte m = 99;

  if ((c > 47) && (c < 58))
  {
    m = c - 22;
  }

  if ((c > 64) && (c < 91))
  {
    m = c - 65;
  }

  if ((c > 96) && (c < 123))
  {
    m = c - 97;
  }

  return m;
}

enum sendMorseStateMachine {
  IDLE,
  SENDING,
  DOT,
  DASH,
  WAIT,
  SPACE
};

void SendMorse()
{
  static sendMorseStateMachine SendMorseSM = IDLE;
  static unsigned long ul_MorseDelay;
  static unsigned long ul_StartMorse;
  static unsigned long ul_EndMorse;
  static byte len;
  static byte chr;
  byte raw;
  char c;
  byte index;
  char s;
  const char *morseptr;

  switch (SendMorseSM)
  {
    case IDLE:

      c = ReadFromBuffer();        // will return 0 if empty
      index = GetMorseIndex(c);    // 0 will return 99, bad letters will return 99

      if (index != 99)
      {
        morseptr = (const char PROGMEM *)MorseCode;

        raw = pgm_read_byte(morseptr + index);  // convert c to uppercase and subtract 65 (A)

        len = (raw & B11100000) >> 5;
        chr = (raw & B00011111);

        SendMorseSM = SENDING;
      }
      break;

    case SENDING:

      if (len > 0)
      {
        LightOn();
        ul_StartMorse = micros();

        if (chr & 1)
        {
          SendMorseSM = DASH;
        }
        else
        {
          SendMorseSM = DOT;
        }
        chr = chr >> 1;
        len--;
      }
      else
      {
        //send white space before returning to IDLE
        SendMorseSM = IDLE;
      }

      break;

    case DOT:
      //Serial.print(F("."));
      ul_MorseDelay = dot;
      SendMorseSM = WAIT;
      break;

    case DASH:
      //Serial.print(F("-"));
      ul_MorseDelay = dash;
      SendMorseSM = WAIT;
      break;

    case WAIT:
      ul_EndMorse = micros();

      if ((ul_EndMorse - ul_StartMorse) > ul_MorseDelay)
      {
        LightOff();
        ul_StartMorse = micros();

        if (len > 0)
        {
          ul_MorseDelay = dot;
        }
        else
        {
          ul_MorseDelay = dash;
        }

        SendMorseSM = SPACE;
      }
      break;

    case SPACE:

      ul_EndMorse = micros();

      if ((ul_EndMorse - ul_StartMorse) > ul_MorseDelay)
      {
        SendMorseSM = SENDING;
      }
      break;
  }

}

void PrintMorse(char c)
{
  byte index;
  byte len;
  byte chr;
  byte raw;

  const char *morseptr = (const char PROGMEM *)MorseCode;

  //only good for A..Z & a..z
  //raw = pgm_read_byte(morseptr + ((c & B11011111) - 65));  // convert c to uppercase and subtract 65 (A)

  index = GetMorseIndex(c);
  raw = pgm_read_byte(morseptr + index);

  len = (raw & B11100000) >> 5;
  chr = (raw & B00011111);

  Serial.println(c);
  Serial.println(index);
  Serial.println(raw);
  Serial.println(len);
  Serial.println(chr);

  if (index != 99)
  {
    for (byte l = 0; l < len; l++)
    {
      char s = '.';
      if (chr & 1)
      {
        s = '-';
      }
      Serial.print(s);
      chr = chr >> 1;
    }
  }

  Serial.println(F(""));
}

void TestMorse()
{
  // test the following range of characters
  // 48.. 57 0..9 -> 26..35
  // 65.. 90 A..Z ->  0..25
  // 97..122 a..z ->  0..25

  //these will output the dash dot combinations for each character
  Serial.println(F("Good Morse"));
  PrintMorse('a');
  PrintMorse('z');
  PrintMorse('A');
  PrintMorse('Z');
  PrintMorse('0');
  PrintMorse('9');

  //these will print no dash dot combinations
  Serial.println(F("Bad Morse"));
  PrintMorse(64);
  PrintMorse(91);
  PrintMorse(96);
  PrintMorse(123);
  PrintMorse(47);
  PrintMorse(58);

  // test the QueueMorse function, this will just place it in a buffer. The loop function will get it out.
  QueueMorse('P');
  QueueMorse('a');
  QueueMorse('r');
  QueueMorse('i');
  QueueMorse('s');
}
