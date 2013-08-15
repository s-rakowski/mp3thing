static const unsigned short int minipaintColors [] = {
  RGB(116,67,34), BLACK, WHITE, RED, RGB(255,127,0), YELLOW, GREEN, BLUE, GREY, GREY//two buttons 
};

void appMinipaint()
{
  LCD_Clear(WHITE);
  short int ii,brush=1;
  unsigned char selectedColor = 1;
  for(ii=0;ii<10;ii++)LCD_Rectangle(ii*24,300,(ii*24)+23,319,minipaintColors[ii]);
  LCD_VerLine(300,319,215,BLACK);
  LCD_DrawString("+    --",203,305,BLACK);
  while(1)
  {
    if(!returnButtonState)break;
    if(touchPressed)
    {
      delay(1);
      if(!touchReadData()) continue;
      if(touchGetY()<300)
      {
        LCD_Rectangle(touchGetX()-(brush>>1),touchGetY()-(brush>>1),touchGetX()+(brush>>1),touchGetY()+(brush>>1),minipaintColors[selectedColor]);
        for(ii=0;ii<10;ii++)LCD_Rectangle(ii*24,300,(ii*24)+23,319,minipaintColors[ii]);
        LCD_VerLine(300,319,215,BLACK);
        LCD_DrawString("+   --",203,305,BLACK);
      }
      else
      {
        while(touchPressed);
        if(touchGetX()<192)selectedColor=touchGetX()/24;
        else if(touchGetX()<216) brush++;
        else if(brush>1) brush--;
      }
    }
  }
}

