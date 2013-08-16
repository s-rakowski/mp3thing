#include <Arduino.h>
//SSD1289 lib
#define LCD_ResetWindow() LCD_SetWindow(0,0,239,319)
#define RGB(red, green, blue)	((unsigned int)( (( red >> 3 ) << 11 ) | \
								(( green >> 2 ) << 5  ) | \
								( blue  >> 3 )))
#define LCD_BUS GPIO_PORTB_DATA_R
#define LCD_RS PC_4
#define LCD_RS_LO HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_4 << 2))) = 0
#define LCD_RS_HI HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_4 << 2))) = GPIO_PIN_4
//cs is connected to gnd
#define LCD_WR PC_5
#define LCD_WR_LO HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_5 << 2))) = 0
#define LCD_WR_HI HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_5 << 2))) = GPIO_PIN_5
//we won't need rd
#define LCD_RESET PC_7
#define LCD_RESET_LO HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0
#define LCD_RESET_HI HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = GPIO_PIN_7

#define LCD_LE PC_6
#define LCD_LOFF HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_6 << 2))) = GPIO_PIN_6
#define LCD_LON HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_6 << 2))) = 0

#define RED  	0xf800
#define GREEN	0x07e0
#define BLUE 	0x001f
#define WHITE	0xffff
#define BLACK	0x0000
#define YELLOW   0xFFE0
#define GREY RGB(150,150,150)

#define FNT(_x) Verdana_font_11[_x]

inline void LCD_WriteData(unsigned short int data) __attribute__((always_inline));
inline void LCD_WriteData(byte higher, byte lower) __attribute__((always_inline));
inline void LCD_WriteIndex(unsigned char index) __attribute__((always_inline));
