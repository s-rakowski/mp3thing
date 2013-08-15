void appPicViewer()
{
  //  FIL file; DIR dir;
  if(!fileDialog(file,dir)) return;
  unsigned char *pix = new unsigned char[480];
  unsigned int n,nump;
  LCD_SetCursor(0,0);
  LCD_WriteIndex(0x22);
  LCD_RS_HI;
  n=480;
  while (n)
  {
    f_read(&file,pix,480,&n);
    for(nump=0;nump<480;nump+=2)
    {
      LCD_WriteData((pix[nump]<<8)|pix[nump|1]);
    }
  }
  f_close(&file);
  delete [] pix;
  delay(100);
  while((!touchPressed)&&returnButtonState);
}


