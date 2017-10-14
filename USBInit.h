class ACMAsyncOper : public CDCAsyncOper
{
  public:
    uint8_t OnInit(ACM *pacm);
};

uint8_t ACMAsyncOper::OnInit(ACM *pacm)
{
  uint8_t rcode;

  // originally set to 3 for DTR=1 RTS=1
  //            set to 1 for DTR=0 RTS=1
  // set DTR=0 RTS=1 to avoid resetting the simulator when plugged in
  rcode = pacm->SetControlLineState(1); //set to 3 for DTR = 1 RTS=1 and (1) for DTR=0 RTS=1

  if (rcode)
  {
    //ErrorMessage<uint8_t>(PSTR("SetControlLineState"), rcode);
    return rcode;
  }

  LINE_CODING  lc;
  lc.dwDTERate  = USBDevBaud;
  lc.bCharFormat  = 0;
  lc.bParityType  = 0;
  lc.bDataBits  = 8;

  rcode = pacm->SetLineCoding(&lc);

  /*
    if (rcode) {
    ErrorMessage<uint8_t>(PSTR("SetLineCoding"), rcode);
    }
  */

  return rcode;
}


USB Usb;
ACMAsyncOper  AsyncOper;
ACM           Acm(&Usb, &AsyncOper);

