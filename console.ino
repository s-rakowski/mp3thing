#define cRows 40
#define cCols 30
#define bgColor BLACK
#define fgColor WHITE
unsigned short int posX=0,posY=0,scrollAmount=320;
unsigned short int s;//,bgColor,fgColor;
void consoleReset()
{
//  bgColor=0; fgColor=0xffff;
  LCD_Clear(bgColor);
  LCD_ResetWindow();
  LCD_SetCursor(0,0);
  scrollAmount=320;
  LCD_HardwareScroll(0);
  posX=0; posY=0;
}

inline void consolePutc(unsigned char c)
{
  if(c=='\b')
  {
    if(posX>0)posX-=8;
  }
  else if(c=='\r')
  {
    posX=0;
  }
  else if(c=='\n')
  {
    posX=0;
    posY+=8;
    if(posY==scrollAmount)
    {
      posY%=320;
      scrollAmount=(scrollAmount+8)%320;
      if(scrollAmount==0)scrollAmount=320;
      LCD_SetWindow(0,0,239,319);
      LCD_HardwareScroll(scrollAmount);
      LCD_SetCursor(posX,posY);
      LCD_SolidFill(1920,bgColor); LCD_SetCursor(posX,posY);
    }
  }
  else
  {
    if(posX>=(cCols*8)-1)
    {
      posX=0;
      posY+=8;
      if(posY==scrollAmount)
      {
        posY%=320;
        scrollAmount=(scrollAmount+8)%320;
        if(scrollAmount==0)scrollAmount=320;
        LCD_SetWindow(0,0,239,319);
        LCD_HardwareScroll(scrollAmount);
        LCD_SetCursor(posX,posY);
        LCD_SolidFill(1920,bgColor); LCD_SetCursor(posX,posY);
      }
    }
      LCD_SetWindow(posX,posY,posX+7,posY+7);
      LCD_WriteIndex(0x22);
      s=8*c;
      for(i=0;i<8;i++)
      {
      LCD_WriteData( ((fontdata_8x8[s+i]>>7)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>6)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>5)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>4)&1) ? fgColor : bgColor );       
      LCD_WriteData( ((fontdata_8x8[s+i]>>3)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>2)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>1)&1) ? fgColor : bgColor );
      LCD_WriteData( ((fontdata_8x8[s+i]>>0)&1) ? fgColor : bgColor );    
      //posY++;
      }
    posX+=8;
  }
}

void consolePuts(const char* _s)
{
  short int len=strlen(_s);
  for(int _i=0;_i<len;_i++)
    consolePutc(_s[_i]);
}

void consolePutTable()
{
  for(char _i=0;_i<255;_i++)
    consolePutc(_i);
}

/*void consoleSetFg(unsigned short int _c)
{
  fgColor=_c;
}

void consoleSetBg(unsigned short int _c)
{
  bgColor=_c;
}

void consoleResetColor()
{
  bgColor=0;
  fgColor=0xffff;
}*/

void consolePutUint(unsigned int __a)
{
  if(__a==0){consolePutc('0'); return;}
  unsigned char str[10], ii=0;
  unsigned int tmp=__a;
  while(tmp>0)
  {
    str[ii++]=tmp%10;
    tmp/=10;
  }
  for(;ii>0;ii--)consolePutc('0'+str[ii-1]);
}

