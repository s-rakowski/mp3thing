//Parts are based on UTouch library, but the version before author switched the license.
//I am not sure how this one should be licensed.

//PIN CONFIGURATIONS
#define CS_PORT GPIO_PORTD_BASE
#define CS_PIN GPIO_PIN_6
#define CLK_PORT GPIO_PORTF_BASE
#define CLK_PIN GPIO_PIN_4
#define MISO_PORT GPIO_PORTF_BASE
#define MISO_PIN GPIO_PIN_3
#define MOSI_PORT GPIO_PORTF_BASE
#define MOSI_PIN GPIO_PIN_2
#define IRQ_PORT GPIO_PORTE_BASE
#define IRQ_PIN GPIO_PIN_2
//-----------------
#define CS_HIGH ROM_GPIOPinWrite(CS_PORT,CS_PIN,CS_PIN)
#define CS_LOW ROM_GPIOPinWrite(CS_PORT,CS_PIN,0)
#define CLK_HIGH ROM_GPIOPinWrite(CLK_PORT,CLK_PIN,CLK_PIN)
#define CLK_LOW ROM_GPIOPinWrite(CLK_PORT,CLK_PIN,0)
//-----------------
unsigned short int dX,dY;
int xcal_a,xcal_b,ycal_a,ycal_b;
//-----------------

void touchInit()
{
  GPIOPinTypeGPIOOutput(CS_PORT,CS_PIN);
  GPIOPinTypeGPIOInput(IRQ_PORT,IRQ_PIN);
  ROM_GPIOPadConfigSet(IRQ_PORT, IRQ_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOPinTypeGPIOInput(MISO_PORT,MISO_PIN);
  GPIOPinTypeGPIOOutput(MOSI_PORT,MOSI_PIN);
  GPIOPinTypeGPIOOutput(CLK_PORT,CLK_PIN);
  CS_HIGH;
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,MOSI_PIN);
}

void _writeByte(unsigned char dat)
{
  unsigned char ii;
  CLK_LOW;
  for(i=7;i>0;i--)
  {
    ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,((dat>>i)&1) ? MOSI_PIN : 0);
    CLK_LOW;
    delayMicroseconds(50);    
    CLK_HIGH;
  }
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,(dat&1) ? MOSI_PIN : 0);
  CLK_LOW;
  delayMicroseconds(50);
  CLK_HIGH;
}

unsigned short int _readData()
{
  unsigned char ii; 
  unsigned short int tmp=0;
  for(ii=0;ii<12;ii++)
  {
    CLK_HIGH;
    delayMicroseconds(50);
    CLK_LOW;
    if(ROM_GPIOPinRead(MISO_PORT,MISO_PIN)) tmp|=1;
    tmp<<=1;
  }
  for(ii=0;ii<4;ii++)
  {
    CLK_HIGH;
    delayMicroseconds(50);
    CLK_LOW;
  }
  return tmp;
}

unsigned short int touch_readXData()
{
  unsigned short int tmp;
  CS_LOW; 
  CLK_LOW;
  _writeByte(0xD0);
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,0);
  delayMicroseconds(50);
  tmp=_readData();
  CS_HIGH;
  return tmp;
}

unsigned short int touch_readYData()
{
  unsigned short int tmp;
  CS_LOW; 
  CLK_LOW;
  _writeByte(0x90);
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,0);
  delayMicroseconds(50);
  tmp=_readData();
  CS_HIGH;
  return tmp;
}
//--------------------------------------
unsigned short int touch_SreadXData()
{
  unsigned short int tmp;
  CS_LOW; 
  CLK_LOW;
  _writeByte(0xD3);
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,0);
  delayMicroseconds(50);
  tmp=_readData();
  CS_HIGH;
  return tmp;
}

unsigned short int touch_SreadYData()
{
  unsigned short int tmp;
  CS_LOW; 
  CLK_LOW;
  _writeByte(0x93);
  ROM_GPIOPinWrite(MOSI_PORT,MOSI_PIN,0);
  delayMicroseconds(50);
  tmp=_readData();
  CS_HIGH;
  return tmp;
}


bool touchReadData()
{
  int _data1,_data2;
  touch_SreadXData();
  dX=( (((int)touch_SreadXData() + (int)touch_SreadXData())>>1) - xcal_a) * (229) / (xcal_b - xcal_a) + 10;

  do
  {
    _data1=touch_SreadYData();
    _data2=touch_SreadYData();
  }
  while (abssub(_data1,_data2)>100);
  touch_readYData();
  if(!touchPressed) return 0;//assume that only aliens and interferences could cause such short presses
  dY=( ( (_data1+_data2)>>1) - ycal_a) * (309) / (ycal_b - ycal_a) + 10;  
}

unsigned short int touchGetX()
{
  return ( (dX>239) ? 239 : dX);
}

unsigned short int touchGetY()
{
  return ((dY>319) ? 319 : dY);
}

void touchSetCalibration(int xa,int xb,int ya,int yb)
{
  xcal_a=xa;
  xcal_b=xb;
  ycal_a=ya;
  ycal_b=yb;
}

