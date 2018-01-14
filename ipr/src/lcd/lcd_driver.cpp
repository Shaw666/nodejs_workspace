
#include <stdio.h>
#include <string.h>
#include "lcd_driver.h"
#include "ascii_8_16.h"

static byte lcd_bits[] = {
    LCD_D0,
    LCD_D1,
    LCD_D2,
    LCD_D3,
    LCD_D4,
    LCD_D5,
    LCD_D6,
    LCD_D7,
};

static void set_data_port_input(void)
{
    for (byte i = 0; i < sizeof(lcd_bits); i ++) {
        pinMode(lcd_bits[i],INPUT_PULLUP);
    }
}


static void set_data_port_output(void)
{
    for (byte i = 0; i < sizeof(lcd_bits); i ++) {
        pinMode(lcd_bits[i], OUTPUT);
    }
}


void lcd_port_init(void)
{
    pinMode(LCD_LIGHT, OUTPUT);  digitalWrite(LCD_LIGHT, HIGH);
    pinMode(LCD_FS,    OUTPUT);  digitalWrite(LCD_FS,    HIGH);
    pinMode(LCD_RST,   OUTPUT);  digitalWrite(LCD_RST,   HIGH);
    pinMode(LCD_CD,    OUTPUT);  digitalWrite(LCD_CD,    HIGH);
    pinMode(LCD_CE,    OUTPUT);  digitalWrite(LCD_CE,    LOW);
    pinMode(LCD_WR,    OUTPUT);  digitalWrite(LCD_WR,    HIGH);
    pinMode(LCD_RD,    OUTPUT);  digitalWrite(LCD_RD,    HIGH);

    for (byte i = 0; i < sizeof(lcd_bits); i++) {
        pinMode(lcd_bits[i], OUTPUT);
    }
}

void lcd_write_cmd(byte data)
{
    LCD_SET_CD;	
	LCD_CLR_WR;    
    for (byte i = 0; i < sizeof(lcd_bits); i ++) {
        digitalWrite(lcd_bits[i], data & 0x01);
        digitalWrite(lcd_bits[i], data & 0x01);
        
        data = data >> 1;
    }      
    LCD_SET_WR;
}


void lcd_write_data(byte data)
{
    LCD_CLR_CD;
	LCD_CLR_WR;    
    for (byte i = 0; i < sizeof(lcd_bits); i++) {
        digitalWrite(lcd_bits[i], data & 0x01);
        digitalWrite(lcd_bits[i], data & 0x01);
        
        data = data >> 1;
    }
	LCD_SET_WR;
}


byte lcd_read_data(void)
{
    byte value = 0X00;
    set_data_port_input();

	LCD_SET_CD;	 
	LCD_SET_WR;
	LCD_CLR_RD;
    
    for (byte i = 0; i < sizeof(lcd_bits); i ++) {
        value |= ( digitalRead(lcd_bits[i]) << i);
    }

    LCD_SET_RD;	
	set_data_port_output();  
    return value;
}

byte lcd_read_reg(byte lcd_reg)
{
	lcd_write_cmd(lcd_reg);
	return lcd_read_data();  	
}


void lcd_clear_text(void)
{
    lcd_write_data(LCD_TEXT_HOME & 0xFF);
    lcd_write_data(LCD_TEXT_HOME >> 8);
    lcd_write_cmd(LCD_SET_ADDRESS_POINTER);

    for(int i = 0; i < LCD_TEXT_SIZE; i++)
    {  
        lcd_write_data(0X00);
        lcd_write_cmd(LCD_DATA_WRITE_AND_INCREMENT);
    }
}

void lcd_clear_attribute(void)
{
    lcd_write_data(LCD_TEXT_ATTRIBUTE_HOME & 0xFF);
    lcd_write_data(LCD_TEXT_ATTRIBUTE_HOME >> 8);
    lcd_write_cmd(LCD_SET_ADDRESS_POINTER);

	/* Graphic RAM is used to Attribute RAM */
    for (int i = 0; i < LCD_TEXT_SIZE; i++)
    {
        lcd_write_data(0X00);
        lcd_write_cmd(LCD_DATA_WRITE_AND_INCREMENT);
    }
}


void lcd_text_xy(byte row, byte col)
{
    unsigned int address = LCD_TEXT_HOME
        +  col
        + (LCD_TEXT_AREA * row);
    
    lcd_write_data(address & 0xFF);
    lcd_write_data(address >> 8);
    lcd_write_cmd(LCD_SET_ADDRESS_POINTER);
}

void lcd_text_attribute_xy(byte row, byte col)
{
    unsigned int address = LCD_TEXT_ATTRIBUTE_HOME
        +  col
        + (LCD_TEXT_AREA * row);
    
    lcd_write_data(address & 0xFF);
    lcd_write_data(address >> 8);
    lcd_write_cmd(LCD_SET_ADDRESS_POINTER);
}

void lcd_write_char(byte row, byte col, char ch, int reverse)
{
	/* write text attribute */
	lcd_text_attribute_xy(row,col);	
	if (reverse){
		lcd_write_data(LCD_TEXT_ATTRIBUTE_REVERSE);	//reverse display
	} else {
		lcd_write_data(LCD_TEXT_ATTRIBUTE_NORMAL);	//nomal display
	}
	lcd_write_cmd(LCD_DATA_WRITE_AND_NONVARIALBE);

	/* write text */
    lcd_text_xy(row, col);
    lcd_write_data(ch);
    lcd_write_cmd(LCD_DATA_WRITE_AND_NONVARIALBE);	
}

void lcd_write_string(byte row, byte col, char * string)
{
    lcd_text_xy(row,col);
    while(*string )
    {
        lcd_write_data(*string - 32);
        lcd_write_cmd(LCD_DATA_WRITE_AND_INCREMENT);
        string ++;
    }
}

void lcd_init(void)
{
    lcd_port_init();
    
    LCD_CLR_RST; 	
    delay(20);
    LCD_SET_RST; 
    delay(20);
    LCD_SET_LIGHT; 
    LCD_CLR_CE;
    LCD_SET_RD;    
}

void lcd_backlight_on() {
    LCD_SET_LIGHT;
}

void lcd_backlight_off() {
    LCD_CLR_LIGHT;
}

void lcd_text_init() {
	LCD_SET_FS;    /* 6 * 8 Font*/
	
	lcd_write_data(LCD_TEXT_ATTRIBUTE_HOME & 0xFF);
    lcd_write_data(LCD_TEXT_ATTRIBUTE_HOME >> 8);
    lcd_write_cmd(LCD_SET_GRAPHIC_HOME_ADDRESS);
	
    lcd_write_data(LCD_TEXT_HOME & 0XFF);
    lcd_write_data(LCD_TEXT_HOME >> 8);
    lcd_write_cmd(LCD_SET_TEXT_HOME_ADDRESS);

    lcd_write_data(LCD_TEXT_AREA);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_TEXT_AREA);

    lcd_write_data(LCD_GRAPHIC_AREA);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_GRAPHIC_AREA);
    
    lcd_write_data(LCD_OFFSET_REGISTER);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_OFFSET_REGISTER);

    lcd_write_cmd(LCD_DISPLAY_MODE  | LCD_GRAPHIC_DISPLAY_ON | LCD_TEXT_DISPLAY_ON);

	/* TEXT Attribute Mode */
    lcd_write_cmd(LCD_MODE_SET | LCD_MODE_TEXT_ATTRIBUTE);	
}

void lcd_graphic_init() {
	
    lcd_write_data(LCD_GRAPHIC_HOME & 0xFF);
    lcd_write_data(LCD_GRAPHIC_HOME >> 8);
    lcd_write_cmd(LCD_SET_GRAPHIC_HOME_ADDRESS);
	
    lcd_write_data(LCD_GRAPHIC_AREA);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_GRAPHIC_AREA);
	
	lcd_write_data(LCD_TEXT_HOME1 & 0XFF);
    lcd_write_data(LCD_TEXT_HOME1 >> 8);
    lcd_write_cmd(LCD_SET_TEXT_HOME_ADDRESS);

    lcd_write_data(LCD_TEXT_AREA);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_TEXT_AREA);
    
    lcd_write_data(LCD_OFFSET_REGISTER);
    lcd_write_data(0x00);
    lcd_write_cmd(LCD_SET_OFFSET_REGISTER);

    lcd_write_cmd(LCD_DISPLAY_MODE  | LCD_GRAPHIC_DISPLAY_ON);

    lcd_write_cmd(LCD_MODE_SET | 0);
}

void lcd_clear_graphic(void) {
    int i;

    lcd_write_data(LCD_GRAPHIC_HOME & 0xFF);
    lcd_write_data(LCD_GRAPHIC_HOME >> 8);
    lcd_write_cmd(LCD_SET_ADDRESS_POINTER);

    for(i = 0; i < LCD_GRAPHIC_SIZE; i++) {
        lcd_write_data(0X00);
        lcd_write_cmd(LCD_DATA_WRITE_AND_INCREMENT);
    }
}

void lcd_draw_picture(byte x, byte y, byte width, byte height, char *pic) {

    unsigned char i, j;
    unsigned char x0 = x/8;
    unsigned char bytes_per_line = width/8;

    for(i = y; i < (height + y ); i ++) {
        for(j = x0; j < (bytes_per_line + x0); j ++) {
            unsigned int address = LCD_GRAPHIC_HOME + j + 40 * i;
            lcd_write_data(address & 0xFF);
            lcd_write_data(address >> 8);
            lcd_write_cmd(LCD_SET_ADDRESS_POINTER);

            lcd_write_data(pic[(i-y)*bytes_per_line + j - x0]);
            lcd_write_cmd(LCD_DATA_WRITE_AND_INCREMENT);
        }
    }
}


void lcd_draw_hz(byte x, byte y, unsigned char *hzstring) {

}

