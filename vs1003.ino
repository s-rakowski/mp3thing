void vsInit()
{
  delay(1);
  ROM_GPIOPinTypeGPIOOutput(VS_RESET_PORT,VS_RESET_PIN);
  ROM_GPIOPinWrite(VS_RESET_PORT,VS_RESET_PIN,0);//hw reset

  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
  ROM_SSIDisable(SSI0_BASE);
  ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
  ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
  ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
  ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_4);
  ROM_GPIOPinTypeGPIOOutput(VS_CS_PORT, VS_CS_PIN);
  ROM_GPIOPinTypeGPIOOutput(VS_DCS_PORT, VS_DCS_PIN);
  ROM_GPIOPinWrite(VS_DCS_PORT,VS_DCS_PIN,VS_DCS_PIN);
  ROM_GPIOPinTypeGPIOInput(VS_DREQ_PORT, VS_DREQ_PIN);

  ROM_SSIClockSourceSet(SSI0_BASE, SSI_CLOCK_SYSTEM);
  ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 200000, 8);
  ROM_SSIEnable(SSI0_BASE);
  unsigned long ttt;
  while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &ttt));
  ROM_GPIOPinWrite(VS_RESET_PORT,VS_RESET_PIN,VS_RESET_PIN);//disable hw reset
  delay(10);
  while(!vsDreq);
  vsWriteReg(SCI_MODE,(1<<SM_RESET)|(1<<SM_SDINEW));
  delay(3);
  while(!vsDreq);
  vsWriteReg(SCI_MODE,(1<<SM_SDINEW));

#ifndef VS1053
  vsWriteReg(SCI_CLOCKF,0xe000);
#else
  vsWriteReg(SCI_CLOCKF,0x8800);
#endif
  delay(10);
  ROM_SSIDisable(SSI0_BASE);
  ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 4000000, 8);
  ROM_SSIEnable(SSI0_BASE);
  delay(4);  
#ifndef VS1053
  for(unsigned short int ii=0;ii<944;ii++)vsWriteReg(spectrum_atab[ii],spectrum_dtab[ii]);
#else
  unsigned short ii=0;
  while (ii<sizeof(vs1053_patch)/sizeof(vs1053_patch[0])) {
    unsigned short addr, n, val;
    addr = vs1053_patch[ii++];
    n = vs1053_patch[ii++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = vs1053_patch[ii++];
      while (n--) {
        vsWriteReg(addr, val);
      }
    } 
    else {           /* Copy run, copy n samples */
      while (n--) {
        val = vs1053_patch[ii++];
        vsWriteReg(addr, val);
      }
    }
  }
#endif
}

unsigned short int vsReadReg(unsigned char regnum)
{
  VS_CS_LOW;
  unsigned short int recv;
  long unsigned int aaa;

  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  HWREG(SSI0_BASE + SSI_O_DR) = 0x03; //read register code
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  recv = (HWREG(SSI0_BASE + SSI_O_DR));

  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  HWREG(SSI0_BASE + SSI_O_DR) = regnum; //register
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  recv = (HWREG(SSI0_BASE + SSI_O_DR));

  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  HWREG(SSI0_BASE + SSI_O_DR) = 0xFF;
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  recv = (HWREG(SSI0_BASE + SSI_O_DR));
  recv<<=8;

  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  HWREG(SSI0_BASE + SSI_O_DR) = 0x00;
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  recv |= (HWREG(SSI0_BASE + SSI_O_DR));

  VS_CS_HIGH;
  return recv;
}

void vsWriteReg(unsigned char regnum, unsigned short int dat)
{
  VS_CS_LOW;
  unsigned long aaa;

  while( !vsDreq );
  HWREG(SSI0_BASE + SSI_O_DR) = 0x02; //write register code
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  aaa = (HWREG(SSI0_BASE + SSI_O_DR));


  HWREG(SSI0_BASE + SSI_O_DR) = regnum; //register
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF));
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  aaa = (HWREG(SSI0_BASE + SSI_O_DR));

  HWREG(SSI0_BASE + SSI_O_DR) = dat>>8;
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  aaa = (HWREG(SSI0_BASE + SSI_O_DR));

  HWREG(SSI0_BASE + SSI_O_DR) = dat&0xFF;
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE));
  aaa = (HWREG(SSI0_BASE + SSI_O_DR));


  while( !vsDreq );
  VS_CS_HIGH;
}

void vsEndPlaying()
{
  vsWriteReg(SCI_VOL,0xfefe);//silence
  vsWriteReg(SCI_MODE,(1<<SM_OUTOFWAV)|(1<<SM_SDINEW));
  unsigned long _a=100001L;
  unsigned char temp;
  while((_a>0)&&(vsReadReg(SCI_HDAT1)!=0)){
    _a--; 
    VS_DCS_LOW; 
    vsWrite(0,temp); 
    VS_DCS_HIGH;
  }
  vsWriteReg(SCI_MODE,(1<<SM_SDINEW));
}


