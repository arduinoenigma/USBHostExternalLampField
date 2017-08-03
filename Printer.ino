byte PrintMaxLine = 8;
byte PrintMaxGroup = 4;
byte PrintGroup = 0;
byte PrintLine = 0;

void InitPrinter()
{
  PrintGroup = 0;
  PrintLine = 0;
}

void SetPrinterGroups(byte groups)
{
  PrintMaxGroup = groups;

  Serial.print(F("[PRINTER:"));
  Serial.print(PrintMaxGroup);
  Serial.println(F("]"));
}

void PrinterFeed()
{
  Serial.println(F("[PRINTER FEED]"));
  PrintGroup = 0;
  PrintLine = 0;
}

void Print(char c)
{
  Serial.print(c);

  PrintGroup++;
  PrintLine++;

  if (PrintLine == PrintMaxLine)
  {
    PrinterFeed();
  }
  else if (PrintGroup == PrintMaxGroup)
  {
    Serial.print(F(" "));
    PrintGroup = 0;
  }

}

