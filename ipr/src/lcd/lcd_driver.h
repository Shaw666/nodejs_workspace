
#ifndef __LCD_DRIVER_H 
#define __LCD_DRIVER_H

#include <stdlib.h>
#include <wirish/wirish.h>

#define LCD_FS       D21    //PB5
#define LCD_LIGHT    D22    //PB6
#define LCD_RST      D47    //PC12
#define LCD_CE       D46    //PC11
#define LCD_CD       D45    //PC10
#define LCD_RD       D24     //PB8
#define LCD_WR       D25     //PB9

/* PD0 - PD7 */
#define LCD_D0       D48
#define LCD_D1       D49
#define LCD_D2       D50
#define LCD_D3       D51
#define LCD_D4       D52
#define LCD_D5       D53
#define LCD_D6       D54
#define LCD_D7       D55


#define LCD_SET_LIGHT    digitalWrite(LCD_LIGHT,HIGH)
#define LCD_CLR_LIGHT    digitalWrite(LCD_LIGHT,LOW)

#define LCD_SET_FS       digitalWrite(LCD_FS,HIGH)
#define LCD_CLR_FS       digitalWrite(LCD_FS,LOW)

#define LCD_SET_RST      digitalWrite(LCD_RST,HIGH) 
#define LCD_CLR_RST      digitalWrite(LCD_RST,LOW)

#define LCD_SET_CD       digitalWrite(LCD_CD,HIGH)
#define LCD_CLR_CD       digitalWrite(LCD_CD,LOW)

#define LCD_SET_CE       digitalWrite(LCD_CE,HIGH)
#define LCD_CLR_CE       digitalWrite(LCD_CE,LOW)

#define LCD_SET_WR       digitalWrite(LCD_WR,HIGH)
#define LCD_CLR_WR       digitalWrite(LCD_WR,LOW)

#define LCD_SET_RD       digitalWrite(LCD_RD,HIGH)
#define LCD_CLR_RD       digitalWrite(LCD_RD,LOW)

#define LCD_NUMBER_OF_LINES          64
#define LCD_PIXELS_PER_LINE          240
#define LCD_FONT_WIDTH               6

#define LCD_GRAPHIC_AREA             (LCD_PIXELS_PER_LINE / LCD_FONT_WIDTH)
#define LCD_TEXT_AREA                (LCD_PIXELS_PER_LINE / LCD_FONT_WIDTH)
#define LCD_GRAPHIC_SIZE             (LCD_GRAPHIC_AREA * LCD_NUMBER_OF_LINES)
#define LCD_TEXT_SIZE                (LCD_TEXT_AREA * (LCD_NUMBER_OF_LINES/LCD_FONT_WIDTH))


#define LCD_TEXT_HOME                0X1400
#define LCD_TEXT_ATTRIBUTE_HOME      0X1700     
#define LCD_OFFSET_REGISTER          2

#define LCD_TEXT_HOME1               0
#define LCD_GRAPHIC_HOME             0X200

#define LCD_SET_CURSOR_POINTER       0x21
#define LCD_SET_OFFSET_REGISTER      0x22
#define LCD_SET_ADDRESS_POINTER      0x24

#define LCD_SET_TEXT_HOME_ADDRESS    0x40
#define LCD_SET_TEXT_AREA            0x41
#define LCD_SET_GRAPHIC_HOME_ADDRESS 0x42
#define LCD_SET_GRAPHIC_AREA         0x43

#define LCD_MODE_SET                 0x80
#define LCD_MODE_EXOR                0x01
#define LCD_MODE_TEXT_ATTRIBUTE      0x04

/* text attribute type */
#define LCD_TEXT_ATTRIBUTE_NORMAL      		0x00
#define LCD_TEXT_ATTRIBUTE_REVERSE	   		0x05
#define LCD_TEXT_ATTRIBUTE_BLINK_NORMAL		0x08
#define LCD_TEXT_ATTRIBUTE_BLINK_REVERSE	0x0D

#define LCD_DISPLAY_MODE            0x90
#define LCD_CURSOR_BLINK_ON         0x01
#define LCD_CURSOR_DISPLAY_ON       0x02
#define LCD_TEXT_DISPLAY_ON         0x04
#define LCD_GRAPHIC_DISPLAY_ON      0x08				

#define LCD_DATA_WRITE_AND_INCREMENT   0xC0
#define LCD_DATA_READ_AND_INCREMENT    0xC1
#define LCD_DATA_WRITE_AND_DECREMENT   0xC2
#define LCD_DATA_READ_AND_DECREMENT    0xC3
#define LCD_DATA_WRITE_AND_NONVARIALBE 0xC4
#define LCD_DATA_READ_AND_NONVARIABLE  0xC5

void lcd_port_init(void);
void lcd_init();

/*-------------- text -----------------*/
void lcd_text_init(void);

void lcd_clear_text(void);
void lcd_clear_attribute(void);

void lcd_text_xy(byte row, byte col);

void lcd_write_char(byte row, byte col,  char ch, int reverse);
void lcd_write_string(byte row, byte col,char * string);

/*-------------- graphic -----------------*/
void lcd_graphic_init(void);
void lcd_clear_graphic(void);

void lcd_draw_picture(byte x, byte y, byte width, byte height, char *pic );

void lcd_draw_hz(byte x, byte y, unsigned char *hzstring);


void lcd_backlight_on();
void lcd_backlight_off();

#endif 


