// this table translates pins 0 thru 6 to a physical arduino pin

//A0..A5 = 14..19
const unsigned char LampFieldPins[] PROGMEM =
{
  A5,
  A4,
  A3,
  A2,
  A0,
  A1
};

// binary encoding of lamp field table:
// BXXXXYYYY where XXXX = binary address of pin to make 1 YYYY = binary address of pin to make 0
// all other pins are set to inputs

// 0001
// 0010
// 0011
// 0100
// 0101
// 0110

const unsigned char LampFieldTable[] PROGMEM =
{
  B00010011,  //  0 - A 1->3
  B01010010,  //  1 - B 5->2
  B01000010,  //  2 - C 4->2
  B00100100,  //  3 - D 2->4
  B01000001,  //  4 - E 4->1
  B01000011,  //  5 - F 4->3
  B00100101,  //  6 - G 2->5
  B01010011,  //  7 - H 5->3
  B00100110,  //  8 - I 2->6
  B00010110,  //  9 - J 1->6
  B01100010,  // 10 - K 6->2
  B00110110,  // 11 - L 3->6
  B01100001,  // 12 - M 6->1
  B01000101,  // 13 - N 4->5
  B01100011,  // 14 - O 6->3
  B00010010,  // 15 - P 1->2
  B00100001,  // 16 - Q 2->1
  B00110100,  // 17 - R 3->4
  B00110010,  // 18 - S 3->2
  B01010001,  // 19 - T 5->1
  B01010100,  // 20 - U 5->4
  B00010101,  // 21 - V 1->5
  B00100011,  // 22 - W 2->3
  B00010100,  // 23 - X 1->4
  B00110001,  // 24 - Y 3->1
  B00110101   // 25 - Z 3->5
};

byte GetRawPin(byte pin)
{
  byte rawpin;
  const char *lampfieldpinsptr;

  lampfieldpinsptr = (const char PROGMEM *)LampFieldPins;

  rawpin = pgm_read_byte(lampfieldpinsptr  + pin);  // convert c to uppercase and subtract 65 (A)

  return rawpin;
}


byte WritePin(byte pin, byte pvalue)
{
  byte rawpin = GetRawPin(pin);

  digitalWrite(rawpin, pvalue);

  return rawpin;
}

byte SetPin(byte pin, byte mode)
{
  byte rawpin = GetRawPin(pin);

  pinMode(rawpin, mode);

  return rawpin;
}


void AllOff()
{
  for (byte i = 0; i < 6; i++)
  {
    SetPin(i, INPUT);
  }
}

void LightKey(char c)
{
  const char *lampcodeptr;

  byte raw;
  byte high;
  byte low;

  c = GetMorseIndex(c);

  if (c < 26)
  {
    lampcodeptr = (const char PROGMEM *)LampFieldTable;

    raw = pgm_read_byte(lampcodeptr  + c);  // convert c to uppercase and subtract 65 (A)

    high = ((raw & B11110000) >> 4) - 1;
    low = ((raw & B00001111)) - 1;

    AllOff();

    SetPin(high, OUTPUT);
    WritePin(high, HIGH);
    SetPin(low, OUTPUT);
    WritePin(low, LOW);
  }

}

