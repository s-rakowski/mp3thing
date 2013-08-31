enum EndReasons{
  FILE_END,
  FILE_NEXT,
  FILE_PREV,
  STOP_PRESSED
};
static const char (*bitrates1[])={"?","32","40","48","56","64","80","96","112","128","160","192","224","256","320","?"};
static const char (*bitrates2[])={"?","8","16","24","32","40","48","56","64","80","96","112","128","144","160","?"};
void appMusicPlayer()
{
  FILINFO inf;
  unsigned int ii,numf=fileDialog(file,dir);
  if(!numf) return;
  for(;;)
  {
    switch(playAudioFile(file))
    {
    case FILE_PREV:
      if(numf==1)f_lseek(&file,0);
      else
      {
        f_opendir(&dir,"");
        numf--;
        for(ii=0;(ii<numf)||(inf.fattrib&AM_DIR);ii++)f_readdir(&dir,&inf);
        f_close(&file);
        if(f_open(&file,inf.fname,FA_READ)!=FR_OK) return;
      }
      break;
    case FILE_END:
    case FILE_NEXT:
      if(f_readdir(&dir,&inf)==FR_OK)
      {
        f_close(&file);
        numf++;
        while((inf.fattrib&AM_DIR))f_readdir(&dir,&inf);
        if(f_open(&file,inf.fname,FA_READ)!=FR_OK) return;
      }
      else return;
      break;
    case STOP_PRESSED:
      f_close(&file);
      return;
      break;
    }
    while(touchPressed);
  }
}

unsigned char playAudioFile(FIL& file)
{
  BYTE buf[32];
  unsigned int n=1,fileSize,playTime,artistPos,titlePos,tempSize;
  unsigned short int s,jj,curPos,lastPos;
  char nameString[256],tempbuf[10]; 
  unsigned short int nameStart=0,nameEnd; 
  byte hasInfo=0,notfound,temp,ii,isPlaying=1,lastVol=map(systemVol,254,0,30,231);
  unsigned char spc[14];//spectrum data
  for(ii=0;ii<14;ii++)spc[ii]=0;
  fileSize=f_size(&file);
  nameString[0]=0;
  //look for ID3
  f_read(&file,tempbuf,3,&n);
  if(!strncmp(tempbuf,"ID3",3))
  {
    f_lseek(&file,6);//skip things
    f_read(&file,tempbuf,4,&n);
    tempSize=tempbuf[3] | (tempbuf[2]<<9) | (tempbuf[1]<<18) | (tempbuf[0]<<27);
    n=2;
    notfound=3;
    while(f_tell(&file)<tempSize&&notfound)
    {
      //TODO: Add skipping large and unused tags like album art
      f_read(&file,tempbuf,4,&n);
      if(!strncmp(tempbuf,"TIT2",4))
      {
        //yay, found title!
        notfound^=2;
        titlePos=f_tell(&file);
      }
      else if(!strncmp(tempbuf,"TPE1",4))
      {
        //yay, found artist!
        notfound^=1;
        artistPos=f_tell(&file);
      }
      f_lseek(&file,f_tell(&file)-3);
    }
    if(!(notfound&1))
    {
      f_lseek(&file,artistPos);
      f_read(&file,tempbuf,4,&n);
      tempSize=tempbuf[3] | (tempbuf[2]<<8) | (tempbuf[1]<<16) | (tempbuf[0]<<24);
      tempSize-=1;
      f_lseek(&file,f_tell(&file)+2);//skip flags
      f_read(&file,tempbuf,1,&n);
      if(tempbuf[0]!=1 && tempSize<255)//not unicode, size ok
      {
        f_read(&file,nameString,tempSize,&n);
        if(nameString[tempSize-1]==0)
        {
          nameString[tempSize-1]='-';
          nameString[tempSize]=0;
        }
        else
        {
          nameString[tempSize]='-';
          nameString[tempSize+1]=0;
        }
      }
    }
    if(!(notfound&2))
    {
      f_lseek(&file,titlePos);
      f_read(&file,tempbuf,4,&n);
      tempSize=tempbuf[3] | (tempbuf[2]<<8) | (tempbuf[1]<<16) | (tempbuf[0]<<24);
      tempSize-=1;
      f_lseek(&file,f_tell(&file)+2);//skip flags
      f_read(&file,tempbuf,1,&n);
      if(tempbuf[0]!=1 && tempSize+strlen(nameString)<256)//not unicode, size ok
      {      
        nameString[tempSize+strlen(nameString)]=0;
        f_read(&file,nameString+strlen(nameString),tempSize,&n);
      }
    }
  }
  if(strlen(nameString))
  {
    for(ii=255;ii>=strlen(nameString);ii--)nameString[ii]=0;
  }
  else for(ii=0;ii<31;ii++)nameString[ii]=0;
  if(strlen(nameString)>30)nameEnd=strlen(nameString)-29; 
  else nameEnd=1;
  vsWriteReg(SCI_VOL,systemVol|(systemVol<<8));
  LCD_ResetWindow(); 
  LCD_Clear(BLACK); //space for ui
  LCD_HorLine(0,239,288,GREEN);
  LCD_VerLine(289,319,59,GREEN);
  LCD_VerLine(289,319,59+60,GREEN);
  LCD_VerLine(289,319,59+60+60,GREEN); 
  drawIcon(21,296,iconPrevious);
  drawIcon(21+180,296,iconNext);
  drawIcon(21+60,296,iconStop);
  drawIcon(21+120,296,iconPause); //draw bottom icons

  LCD_VerLine(262,281,7,GREEN);
  LCD_VerLine(262,281,232,GREEN);
  LCD_HorLine(7,232,262,GREEN);
  LCD_HorLine(7,232,281,GREEN);

  LCD_VerLine(236,255,29,GREEN);
  LCD_VerLine(236,255,232,GREEN);
  LCD_VerLine(238,253,map(systemVol,254,0,30,231),GREEN);
  LCD_HorLine(29,232,236,GREEN);
  LCD_HorLine(29,232,255,GREEN);
  drawIcon(7,238,iconSound);

  f_lseek(&file,0);
  VS_CS_HIGH;
  VS_DCS_LOW;
  while(1)
  {
    if(isPlaying&&vsDreq)
    {
      f_read(&file,buf,32,&n);
      if(!n)
      { 
        VS_DCS_HIGH;
        vsEndPlaying();
        return FILE_END;
      }
      for(ii=0;ii<n;ii++)
      {
        vsWrite(buf[ii],temp);
      }
      continue;
    }
    //redraw UI
    if(millis()%1000==0)
    {
      for(j=0;j<30;j++)
      {
        LCD_SetWindow(j<<3,180,(j<<3)+7,187);
        LCD_WriteIndex(0x22);
        if(j+nameStart);
        s=8*nameString[j+nameStart];
        for(i=0;i<8;i++)
        {
          LCD_WriteData( ((fontdata_8x8[s+i]>>7)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>6)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>5)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>4)&1) ? GREEN : BLACK );       
          LCD_WriteData( ((fontdata_8x8[s+i]>>3)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>2)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>1)&1) ? GREEN : BLACK );
          LCD_WriteData( ((fontdata_8x8[s+i]>>0)&1) ? GREEN : BLACK );    
        }
      }
      nameStart=(nameEnd==1) ? 0 : (nameStart+1)%nameEnd;
    }
    if(millis()%40==0){
      VS_DCS_HIGH;
      if(!hasInfo)
      {
        s=vsReadReg(SCI_HDAT1);
        if(s)
        {
          hasInfo=1;
          if(s==0x7665)LCD_DrawString("WAV",10,200,GREEN);
          else if(s==0x574D)
          {
            LCD_DrawString("WMA",10,200,GREEN);
            s=vsReadReg(SCI_HDAT0);
            s/=128; //get kbps
            ii=0;
            if(s>99){tempbuf[ii++]='0'+s/100; s%=100;}
            if(s>9){tempbuf[ii++]='0'+s/10; s%=10;}
            tempbuf[ii++]='0'+s;
            tempbuf[ii++]=0;
            LCD_DrawString(tempbuf,45,200,GREEN);
          }
          else if(s==0x4D54)LCD_DrawString("MID",10,200,GREEN);
          else
          {
            LCD_DrawString("MP3",10,200,GREEN);
            ii=(s>>3)&3;
            s=vsReadReg(SCI_HDAT0);
            switch((s>>6)&3)
            {
              case 0: LCD_DrawString("Stereo",75,200,GREEN); break;
              case 1: LCD_DrawString("Joint Stereo",75,200,GREEN); break;
              case 2: LCD_DrawString("Dual Channel",75,200,GREEN); break;
              case 3: LCD_DrawString("Mono",75,200,GREEN); break;
            }
            s>>=12;
            s&=15;
            LCD_DrawString((ii==3) ? bitrates1[s] : bitrates2[s],45,200,GREEN);
          }
        } 
      }
      //draw spectrum bars
      vsWriteReg(SCI_WRAMADDR,0x1384);
      for(ii=0;ii<14;ii++)
      {
        temp=(vsReadReg(SCI_WRAM)&63)<<1;
        if(temp<spc[ii])
          LCD_Rectangle(1+(ii*17),179-spc[ii],17+(ii*17),179-temp,BLACK);
        else
          LCD_Rectangle(1+(ii*17),179-temp,17+(ii*17),179-spc[ii],GREEN);
        spc[ii]=temp;
      }    
      VS_DCS_LOW;
    }
    //draw position bar
    curPos=map(f_tell(&file)>>3,0,fileSize>>3,0,232); //Max file size is still ~70MB. At least I don't have that large audio files.
    if(curPos!=lastPos)
    {
      LCD_VerLine(264,279,8+lastPos,BLACK);
      lastPos=curPos;
      LCD_VerLine(264,279,8+lastPos,GREEN);
    }
    if(touchPressed&&touchReadData())
    {
      if(touchGetY()>287)
      {
        switch(touchGetX()/60)
        {
        case 2: //play/pause
          if(isPlaying)
          {
            isPlaying=0;
            drawIcon(21+120,296,iconPlay);
          }
          else
          {
            isPlaying=1;
            drawIcon(21+120,296,iconPause);
          }
          break;
        case 0: //prev
          VS_DCS_HIGH;
          vsEndPlaying();
          return FILE_PREV;
          break;
        case 3: //next
          VS_DCS_HIGH;
          vsEndPlaying();
          return FILE_NEXT;
          break;
        case 1:
          VS_DCS_HIGH;
          vsEndPlaying();
          return STOP_PRESSED;
          break;
        }
        while(touchPressed);
      }
      else if(touchGetY()>251&&touchGetY()<281&&touchGetX()>7&&touchGetX()<232) //position bar
      {
        f_lseek(&file,map(touchGetX()-8,0,223,0,fileSize>>3)<<3);
        delay(1);
      }
      else if(touchGetY()>236&&touchGetY()<255&&touchGetX()>29&&touchGetX()<232) //volume bar
      {
        systemVol=map(touchGetX()-30,0,201,254,0);
        VS_DCS_HIGH;
        vsWriteReg(SCI_VOL,(systemVol<<8)|systemVol);
        VS_DCS_LOW;
        LCD_VerLine(238,253,lastVol,BLACK);      
        LCD_VerLine(238,253,touchGetX(),GREEN);
        lastVol=touchGetX();
        delay(1);
      }
    }
  }
}











