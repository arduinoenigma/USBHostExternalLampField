byte PrintMaxLine = 16;
byte PrintMaxGroup = 4;
byte PrintGroup = 0;
byte PrintLine = 0;
byte PrintedGroups = 0;

void InitPrinter()
{
  PrintGroup = 0;
  PrintLine = 0;
}

void SetPrinterGroups(byte groups)
{
  PrintMaxGroup = groups;

  //Serial.print(F("[PRINTER:"));
  //Serial.print(PrintMaxGroup);
  //Serial.println(F("]"));
}

void PrinterFeed()
{
  //Serial.println(F("[PRINTER FEED]"));
  PrintGroup = 0;
  PrintLine = 0;
  PrintedGroups = 0;

  printer.feed(2);
}

void Print(char c)
{
  //Serial.print(c);
  printer.print(c);

  PrintGroup++;
  PrintLine++;

  if (PrintLine == PrintMaxLine)
  {
    PrinterFeed();
  }
  else if (PrintGroup == PrintMaxGroup)
  {
    //Serial.print(F(" "));
    printer.print(F(" "));
    PrintGroup = 0;
    PrintedGroups++;
  }

  if (((PrintedGroups == 2) && ((PrintMaxGroup == 5) || (PrintMaxGroup == 6))) || ((PrintedGroups == 3) && (PrintMaxGroup == 4)))
  {
    PrinterFeed();
  }
}

