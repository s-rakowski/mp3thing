void drawMenuPos(unsigned short int y)
{
  LCD_ModeDR();
  unsigned char jj,kk;
  LCD_SetWindow(0,y*20,239,(y*20)+19);
  LCD_WriteIndex(0x22);
  for(jj=0;jj<240;jj++)
  {
    for(kk=0;kk<40;kk+=2)
    {
      LCD_WriteData((Gradient20[kk|1]<<8)|Gradient20[kk]);
    }
  }
  LCD_ModeRD();
}

void drawIcon(byte x, unsigned int y, const unsigned char *icondata)//draws 16x16 icon
{
  LCD_SetWindow(x,y,x+15,y+15);
  LCD_WriteIndex(0x22);
  for(unsigned short int ii=0; ii<512;ii+=2)
    LCD_WriteData((icondata[ii]<<8)|icondata[ii|1]);
  LCD_ResetWindow();
}

unsigned int fileDialog(FIL &rtn_file, DIR &dir)
{
  FRESULT res;
  FILINFO fno;
  //DIR dir;
  unsigned char ii,chosen=0,endPage=0;
  unsigned int jj,startPos=0;
  LCD_SetCursor(0,300);
  LCD_SolidFill(4800,GREY);
  LCD_DrawString(fileDialogUp, 40,304,BLACK);
  LCD_DrawString(fileDialogDown, 175,304,BLACK);
  while(!chosen)
  {
    LCD_SetWindow(0,0,239,299); 
    LCD_Clear(WHITE); 
    LCD_ResetWindow();
    res = f_opendir(&dir,"");
    if (res == FR_OK)
    {
      for(jj=0;jj<startPos;jj++)res = f_readdir(&dir,&fno); //TODO: optimize this
      for(ii=0;ii<15;ii++)
      {
        res=f_readdir(&dir,&fno);
        if(res!=FR_OK||fno.fname[0]==0){
          endPage=1; 
          break;
        }
        drawMenuPos(ii);
        if(fno.fattrib & AM_DIR) drawIcon(2,2+ii*20,iconFolder);
        else drawIcon(2,2+ii*20,iconFile);
        LCD_DrawString(fno.fname,19,ii*20+4,BLACK);
      }
    }
    else LCD_Clear(RED);
    while(!touchPressed){
      if(!returnButtonState) return 0;
    }
    touchReadData();
    while(touchPressed)delay(100);
    if(touchGetY()>299)
    {
      if(touchGetX()>119)//down button
      {
        if(!endPage)startPos+=15;
      }
      else //up button
      {
        if(startPos>0)startPos-=15,endPage=0;
      }
    }
    else if(touchGetY()<(20*(ii+1)))
    {
      res=f_opendir(&dir,"");
      for(jj=0;jj<=(startPos+(touchGetY()/20));jj++)res = f_readdir(&dir,&fno); //I think this can be done better
      if(res==FR_OK&&(!(fno.fattrib&AM_DIR)))chosen=1;
      else if(res==FR_OK){
        startPos=0; 
        endPage=0; 
        f_chdir(fno.fname);
      }
    }
  }
  if(f_open(&rtn_file,fno.fname,FA_READ)==FR_OK) return jj; 
  else return 0;
}

