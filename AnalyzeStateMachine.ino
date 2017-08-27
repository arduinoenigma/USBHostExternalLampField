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
  CL,
  PF,
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

byte SendOneTilde = 1;

void Analyze(char inKey)
{
  static verboseStateMachine VerboseSM = NL;
  static byte SteckerCounter = 0;
  bool reset = false;

  uint8_t data;
  uint8_t rcode;

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
          LightOff();
          AllOff();
        }
      }

      if (inKey == 'e')
      {
        VerboseSM = E;
      }
      break;

    case E:

      if (SendOneTilde)
      {
        // send signal to enigma simulator to supress lampfield
        // ~ to disable lampfield and printer   @ to disable printer

        SendOneTilde = 0;
        //data = '@';   // @ disables printer only
        data = '~';   // ! disables lampfield and printer

        rcode = Acm.SndData(1, &data);
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
      // found "er", could be "Stecker>" or "Printer:" or "PrinterFeed"

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
        VerboseSM = CL;
      }
      // look for "PrinterFeed" string
      else if (inKey == 'F')
      {
        VerboseSM = PF;
      }
      else
      {
        reset = true;
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

    case CL:

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

