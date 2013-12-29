#define abssub(__a,__b) ( (__a>__b) ? (__a-__b) : (__b-__a) )
void appSlide()
{
//  FIL file; DIR dir;
  if(!fileDialog(file,dir)) return;
  unsigned char *pixbuf = new unsigned char[480];//one line
  unsigned short int ii,jj,y1,y2;
  unsigned short int scrollData=320;
  unsigned int n,upPos=0,maxPos=f_size(&file)-153600; //needed to figure out when to not scroll down
  LCD_ResetWindow();
  LCD_SetCursor(0,0);
  LCD_WriteIndex(0x22);

  for(ii=0;ii<320;ii++)//fill the screen first
  {
    f_read(&file,pixbuf,480,&n);
    if(n<480){
      ii=400; //check if file isn't too small
      break;
    }
    for(jj=0;jj<480;jj+=2)
    {
      LCD_WriteData((pixbuf[jj]<<8)|pixbuf[jj|1]);
    }
  }
  while(1)
  {
    if(!returnButtonState) break;
    if(touchPressed)
    {
      if(!touchReadData()) continue;
      y1=touchGetY();
      delay(20);
      if( !(touchPressed && touchReadData()))
      {
        delay(100); 
        continue;
      }
      y2=touchGetY();
      if(abssub(y1,y2)<5)continue;
      if(y1>y2 && upPos<maxPos)//go down
      {
        LCD_SetCursor(0,scrollData);
        if(upPos+(y1-y2)*480>maxPos) y1=y2+(maxPos-upPos)/480;
        f_lseek(&file,upPos+153600);
        scrollData=(scrollData+(y1-y2))%320;
        if(scrollData==0)scrollData=320;
        LCD_HardwareScroll(scrollData);
        LCD_WriteIndex(0x22);
        for(ii=0;ii<(y1-y2);ii++)//fill some lines
        {
          f_read(&file,pixbuf,480,&n);
          for(jj=0;jj<480;jj+=2)
          {
            LCD_LOFF;
            LCD_BUS = pixbuf[jj|1]; //higher byte first
            LCD_LON;
            LCD_BUS = pixbuf[jj];
            LCD_WR_LO;
            LCD_WR_HI;
          }
        }
        upPos+=(y1-y2)*480;
      }
      else if (y1<y2 && upPos>0)//go up
      {
        if((y2-y1)*480>upPos) y2=y1+upPos/480;
        upPos-=(y2-y1)*480;
        f_lseek(&file,upPos);
        scrollData=(320+scrollData-(y2-y1))%320;
        LCD_SetCursor(0,scrollData);
        if(scrollData==0)scrollData=320;
        LCD_HardwareScroll(scrollData);
        LCD_WriteIndex(0x22);
        for(ii=0;ii<(y2-y1);ii++)//fill some lines
        {
          f_read(&file,pixbuf,480,&n);
          for(jj=0;jj<480;jj+=2)
          {
            LCD_LOFF;
            LCD_BUS = pixbuf[jj|1]; //higher byte first
            LCD_LON;
            LCD_BUS = pixbuf[jj];
            LCD_WR_LO;
            LCD_WR_HI;
          }
        }
      }
    }
  }  
  f_close(&file);
  LCD_HardwareScroll(0);
  delete [] pixbuf;
}





