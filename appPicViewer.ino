void appPicViewer()
{
  //  FIL file; DIR dir;
  //This is useless at the moment!
  if(!fileDialog(file,dir)) return;
  unsigned char *pix = new unsigned char[480];
  unsigned int n,nump;
  unsigned long time=millis();
  //LCD_SetCursor(0,0);
  LCD_SetWindow(0,0,239,179);
  LCD_WriteIndex(0x22);
  n=480;
  while (n)
  {
    f_read(&file,pix,480,&n);
    for(nump=0;nump<n;nump+=2)
    {
      LCD_WriteData(pix[nump],pix[nump|1]);
    }
  }
  f_close(&file);
  delete [] pix;
  consoleReset();
  consolePutUint(millis()-time);
  delay(100);
  while((!touchPressed)&&returnButtonState);
  LCD_ResetWindow();
}


