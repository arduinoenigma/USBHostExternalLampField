/*
  state machine
  detect new line, wait for er> twice, then output next character, new line at any time returns to initial state:
  this state machine will call QueueMorse(inKey) to put the desired character in a queue of morse to be sent out
  the minimal test string to flash X is er>er>X

  Q>Stecker>Q>ETW>Q>R1>V>R2>Y>R3>C>R4>Y>UKW>G>R4>U>R3>R>R2>G>R1>C>ETW>C>Stecker>C
  W>Stecker>W>ETW>W>R1>O>R2>M>R3>O>R4>D>UKW>Q>R4>M>R3>C>R2>P>R1>G>ETW>G>Stecker>G
  E>Stecker>E>ETW>E>R1>M>R2>W>R3>B>R4>E>UKW>A>R4>R>R3>X>R2>I>R1>C>ETW>C>Stecker>C
  A>Stecker>A>ETW>A>R1>N>R2>T>R3>P>UKW>U>R3>R>R2>G>R1>O>ETW>O>Stecker>O
  S>Stecker>S>ETW>S>R1>W>R2>F>R3>G>UKW>Y>R3>O>R2>Y>R1>T>ETW>T>Stecker>T
  D>Stecker>D>ETW>D>R1>S>R2>Z>R3>J>UKW>B>R3>W>R2>M>R1>C>ETW>C>Stecker>C

  A>Stecker>A>ETW>A>R1>A>R2>A>R3>A>R4>A>UKW>A>R4>A>R3>A>R2>A>R1>A>ETW>A>Stecker>B
  A>Stecker>A>ETW>A>R1>A>R2>A>R3>A>UKW>A>R3>A>R2>A>R1>A>ETW>A>Stecker>Z
*/

enum verboseStateMachine {
  NL,
  E,
  R,
  GT,
  PG,
  PF,
  SB,
  DONE
};

unsigned long ul_LampOn = 0;
unsigned long ul_LampOff = 0;
byte IsLampOn = 0;
byte IsShortPress = 0;

void MininumLightTime()
{
  unsigned long ul_TimeOn;

  if (IsShortPress)
  {
    ul_TimeOn = micros() - ul_LampOn;

    if ( ul_TimeOn > ((unsigned long)1000000))
    {
      IsShortPress = 0;
      LightOff();
      AllOff();
    }
  }
}

// list of valid commands that the Analyze function recognizes
//er>er>X       // output X to the printer and light board until <CR><LF> is received, if <CR><LF> is received immediately, X is lit for a fixed amount of time
//<CR><LF>      // turn X off (no effect on printer)
//Printer:4     // sets printer groups, every N characters a space is printed, if 0, no spaces are printed, valid values are 0,4,5,6
//PrinterFeed   // empties the print buffer and sends Line Feed to the thermal printer
//Brighter=+    // increments PWM brightness value by 1
//Brighter=-    // decrements PWM brightness value by 1
//Brighter=}    // increments PWM brightness value by 10
//Brighter={    // decrements PWM brightness value by 10
//Brighter=?    // ? and any other character not listed above, sends the PWM brightness value to the slave device as three digits ie: 025

byte SendOneTilde = 1;
char LastKey = 0;

void Analyze(char inKey)
{
  static verboseStateMachine VerboseSM = NL;
  static byte SteckerCounter = 0;
  bool reset = false;

  if (inKey == 13)
  {
    SteckerCounter = 0;
    reset = true;
  }

  switch (VerboseSM)
  {
    case NL:

      if (IsLampOn)
      {
        IsLampOn = 0;
        ul_LampOff = micros();

        // 5000 for v1.7 vs 90000 for v1.8
        if ((ul_LampOff - ul_LampOn) < 5000)
        {
          IsShortPress = 1;
        }
        else
        {
          //found you
          //if letter change command is sent while save is blinking, it turns light off
          if (autoSaveDone == 0)
          {
            LightOff();
            AllOff();
          }
        }
      }
      autoSaveDone = 0;

      if (inKey == 'e')
      {
        VerboseSM = E;
      }
      break;

    case E:

      if (SendOneTilde)
      {
        SendOneTilde = 0;
        // send signal to enigma simulator to supress lampfield
        // ~ to disable lampfield and printer   @ to disable printer
        SendUSBChar('~');
      }

      if (inKey == 'r')
      {
        VerboseSM = R;
      }
      else
      {
        reset = true;
      }
      break;

    case R:

      // found "er", could be "Stecker>" or "Printer:" or "PrinterFeed" or "Brighter="

      // look for "Stecker>" string
      if (inKey == '>')
      {
        if (SteckerCounter == 0)
        {
          SteckerCounter++;
          reset = true; // found first er>, go back and look for it again
        }
        else
        {
          // found er> for a second time, output next character if not newline
          VerboseSM = GT;
        }
      }
      // look for "Printer:" string
      else if (inKey == ':')
      {
        VerboseSM = PG;
      }
      // look for "PrinterFeed" string
      else if (inKey == 'F')
      {
        VerboseSM = PF;
      }
      // look for "Brighter=" string
      else if (inKey == '=')
      {
        VerboseSM = SB;
      }
      else
      {
        reset = true;
      }

      if (autoSaveTimer)
      {
        autoSaveTimer = AUTOSAVEDELAY;
      }

      break;

    case GT:

      // Action
      QueueMorse(inKey);
      Print(inKey);
      LightKey(inKey);

      IsLampOn = 1;
      ul_LampOn = micros();

      VerboseSM = DONE;
      break;

    case PG:

      SetPrinterGroups(inKey - '0');

      VerboseSM = DONE;
      break;

    case PF:

      if (inKey == 'e')
      {
        PrinterFeed();
      }

      VerboseSM = DONE;
      break;

    case SB:

      if (inKey == '+')
      {
        if (LampFieldData.PWMDuty < 255)
        {
          LampFieldData.PWMDuty++;
          autoSaveTimer = AUTOSAVEDELAY;
        }
      }

      if (inKey == '-')
      {
        if (LampFieldData.PWMDuty > 1)
        {
          LampFieldData.PWMDuty--;
          autoSaveTimer = AUTOSAVEDELAY;
        }
      }

      if (inKey == '}')
      {
        if (LampFieldData.PWMDuty < 246)
        {
          LampFieldData.PWMDuty += 10;
          autoSaveTimer = AUTOSAVEDELAY;
        }
      }

      if (inKey == '{')
      {
        if (LampFieldData.PWMDuty > 10)
        {
          LampFieldData.PWMDuty -= 10;
          autoSaveTimer = AUTOSAVEDELAY;
        }
      }

      // unrecognized character, like ? will do nothing, but output the current brightness value
      //Serial.println(LampFieldData.PWMDuty);
      SendUSBShortInt(LampFieldData.PWMDuty);

      VerboseSM = DONE;
      break;

    case DONE:

      SendOneTilde = 1;
      // stay here until new line is detected
      break;
  }

  if (reset)
  {
    VerboseSM = NL;
  }
}

void SendUSBShortInt(byte val)
{
  char buff[3];
  uint8_t data;
  uint8_t rcode;

  for (byte i = 0; i < 3; i++)
  {
    buff [i] = 0;
  }

  for (byte i = 0; i < 3; i++)
  {
    buff [2 - i] = '0' + val - (val / 10) * 10;

    val = val / 10;
  }

  for (byte i = 0; i < 3; i++)
  {
    data = buff[i];
    rcode = Acm.SndData(1, &data);
  }
}

void SendUSBChar(char c)
{
  uint8_t data;
  uint8_t rcode;

  data = c;
  rcode = Acm.SndData(1, &data);
}
