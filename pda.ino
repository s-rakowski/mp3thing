#define returnButtonState (ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0))
#define touchPressed (! (ROM_GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_2)) )
#include <ff.h>
#include "font_8x8.c"
#include "lcd.h"
#include "ff.h"
#include "diskio.h"
#include "icons.h"
#include "vs1003.h"
#include "gradients.h"
#include <inc/hw_gpio.h>
#include <inc/hw_ints.h>
#include <inc/hw_timer.h>
#include <inc/lm4f120h5qr.h>
#include <inc/hw_ssi.h>
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/eeprom.h"
#include "driverlib/ssi.h"
//----------------------------
#include "stringData.h"
FATFS fatfs;
FIL file; DIR dir;//shared
#define numApps 7
void (*appList[numApps])() = {
  appCalibrate,appMinipaint,appPicViewer,appSlide,appMusicPlayer,appPlasma,appTest2};
static const char (*appNames[numApps]) = {
  appCalibrateName,appMinipaintName,appPicViewerName,appSlideName,appMusicPlayerName,appPlasmaName,appTest2Name};

byte j,k,systemVol=0;
unsigned int i;
void FatInt()
{
  HWREG(TIMER0_BASE + TIMER_O_ICR) = TIMER_TIMA_TIMEOUT;
  disk_timerproc();
}
void setup()
{
  //Set up LCD and touch
  LCD_Init();
  consoleReset();
  Serial.begin(115200);
  Serial.println(sizeof(bool));
  consolePuts(initvs_str);
  vsInit();
  consolePuts(inittouch_str);
  touchInit();

  //Set up return button(SW2)
  ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0);
  ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  consolePuts(initsd_str);

  //Set up the timer for FatFs
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  TimerIntRegister(TIMER0_BASE,TIMER_A,FatInt);
  ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet() / 100 )-1);
  ROM_IntEnable(INT_TIMER0A);
  ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);

  //Initialize SD card
  BYTE d; 
  d=disk_initialize(0);
  if(d!=FR_OK)
  {
    consolePuts(sdfail_str);
    while(1);
  }
  d = f_mount( 0, &fatfs );
  if(d!=FR_OK)
  {
    consolePuts(mountfail_str);
    while(1);
  }
  //Check for touch calibration data in internal EEPROM
  consolePuts(initcal_str);
  long unsigned int calD[5];
  ROM_EEPROMRead(calD,0,20);
  if(calD[0]==0xC0FFEE)//:D
    touchSetCalibration(calD[1],calD[2],calD[3],calD[4]);
  else appCalibrate();
}

void loop()
{
  LCD_ResetWindow();
  LCD_Clear(WHITE);
  for(i=0;i<numApps;i++)
  {
    LCD_ModeDR();
    LCD_SetWindow(0,i*20,239,(i*20)+19);
    LCD_WriteIndex(0x22);
    for(j=0;j<240;j++)
    {
      for(k=0;k<40;k+=2)
      {
        LCD_WriteData(Gradient20[k|1],Gradient20[k]);
      }
    }
    LCD_ModeRD();
    LCD_ResetWindow();
    LCD_DrawString(appNames[i],19,(i*20)+4,BLACK);
  }
  while(!touchPressed);
  touchReadData();
  while(touchPressed)delay(100);

  if((touchGetY()/20)<numApps)appList[touchGetY()/20]();
}






