#include <stdlib.h>
#include <math.h>
#include <wirish/wirish.h>
#include <libmaple/nvic.h>
#include <string.h>
#include <HardwareSerial.h>
#include <HardwareTimer.h>
#include "lcd_driver.h"
#include "chars.h"
#include "ascii_8_8.h"
#include "ascii_8_16.h"
#include "conwin_logo.h"

#define BTN_PIN_LEFT  D76
#define BTN_PIN_OK    D75
#define BTN_PIN_DOWN  D77
#define BTN_PIN_UP    D78
#define BTN_PIN_BACK  D74
#define BTN_PIN_RIGHT D79

//背光关闭时间
#define BACKLIGHT_TIME (5 * 60)

bool connected      = false; 
unsigned int backlight_time = BACKLIGHT_TIME;

typedef enum {
    BTN_NOP = -1,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_OK,
    BTN_BACK,
} Btn_code;

Btn_code btn_last;
byte btn_pin_state = 0;
byte btn_pin_bits[] = {
    BTN_PIN_UP,
    BTN_PIN_DOWN,
    BTN_PIN_LEFT,
    BTN_PIN_RIGHT,
    BTN_PIN_OK,
    BTN_PIN_BACK,
};
short btn_timer[6] = {-1};

#define SCREEN_WIDTH  40
#define SCREEN_HEIGHT 8
#define CURSOR_ON     0x01

#define MODE_INVERSE 0x0100

#define word short

word screen_char_mode = 0;
word screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH]   = {0};
word screen_buffer_B[SCREEN_HEIGHT][SCREEN_WIDTH] = {0};
byte cursor_row  = 0;
byte cursor_col  = 0;
byte cursor_flag = 0;
byte cursor_stat = 0;

byte cursor_cnt  = 2;

//按键发送循环缓冲
Btn_code btn_queue_buf[32];
byte     btn_queue_buf_head = 0;
byte     btn_queue_buf_tail = 0;

void btn_init_pins() {

    for (byte i = 0; i < sizeof(btn_pin_bits); i++) {
        pinMode(btn_pin_bits[i], INPUT_PULLUP);
    }
}

void lcd_move_to(byte row, byte col) {
    lcd_text_xy(row,col);
}

void lcd_draw_ascii_68_at(byte row, byte col, word c){
	char ch   = c & 0xFF;
    word mode = c & 0xFF00;
    if (ch < ' ') {
        ch = ' ';
    } 
    ch = ch - ' ';
	lcd_write_char(row, col, ch, mode & MODE_INVERSE);      
};


void lcd_timer_handler() {
    static unsigned int timer_cnt = 0;
    if (++timer_cnt >= 10) {
        timer_cnt = 0;
        if (backlight_time) {
            backlight_time --;
            lcd_backlight_on();
        } else {
            lcd_backlight_off();
        }
    }
    
    cursor_cnt--;
    for (byte row = 0; row < SCREEN_HEIGHT; row++) {
        byte col = 0;
        while (col < SCREEN_WIDTH) {
            if (screen_buffer[row][col] == screen_buffer_B[row][col]) {
                col++; continue;
            }
            while (col < SCREEN_WIDTH
                   && (screen_buffer[row][col]
                       != screen_buffer_B[row][col])) {
                word c = screen_buffer[row][col];
                lcd_draw_ascii_68_at(row , col, c);

                screen_buffer_B[row][col] = screen_buffer[row][col];
                col++;
            }
        }
    }
};

void lcd_draw_ascii_at(byte row, byte col, char ch, int reverse) {
    if (ch <= ' ') {
        ch = ' ';
    } else {
        ch = ch - ' ';
    }

	lcd_write_char(row, col, ch, reverse);
};

void scr_move_to(byte row, byte col) {
    cursor_row = row;
    cursor_col = col;
};

void scr_cursor_on() {
    cursor_flag = cursor_flag | CURSOR_ON;
    cursor_stat = cursor_stat & 0xFE;
};

void scr_cursor_off() {
    cursor_flag = cursor_flag & (~CURSOR_ON);
    cursor_stat = cursor_stat & 0xFE;
};

void scr_clean() {
    
    for (int row = 0; row < SCREEN_HEIGHT; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            screen_buffer[row][col]   = (word)' ' & (~MODE_INVERSE);
        }
    }
    cursor_col = 0;
    cursor_row = 0;
};

void scr_scroll_up() {
    for (int row = 0; row < SCREEN_HEIGHT - 1; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            screen_buffer[row][col] = screen_buffer[row + 1][col];
        }
    }
    for (int col = 0; col < SCREEN_WIDTH; col++) {
        screen_buffer[SCREEN_HEIGHT - 1][col] = (word)' ';
    }
};

void scr_print(const char ch) {
    if (cursor_col >= SCREEN_WIDTH) { return; };
    screen_buffer[cursor_row][cursor_col] =
        screen_char_mode | ch;
    cursor_col++;
};

void scr_print(const char *str) {
    if (!str) { return; };
    if (cursor_col >= SCREEN_WIDTH) { return; };
    for (int i = 0; i < (int)strlen(str); i++) {
        scr_print(str[i]);
    }
};

void scr_println() {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= SCREEN_HEIGHT) {
        scr_scroll_up();
        cursor_row = SCREEN_HEIGHT - 1;
    }
};

void scr_println(const char *str) {
    scr_print(str);
    scr_println();
};
    
void setup() {
    Serial1.begin(115200);
    Serial2.begin(115200);
    Serial2.println("hi this is maple");
    pinMode(BOARD_LED_PIN, OUTPUT);
    btn_init_pins();
    lcd_init();
	lcd_text_init();
    lcd_clear_text();

    lcd_graphic_init();
    lcd_clear_attribute();
    lcd_clear_graphic();

    lcd_draw_picture(72, 19, LOGO_WIDTH, LOGO_HEIGHT, (char *)pic_conwin_logo);
	
    nvic_irq_set_priority(NVIC_USART2,0);
    
	Timer2.pause();
    Timer2.setPeriod(100 * 1000);  
    Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer2.setCompare(TIMER_CH1, 1);
    Timer2.attachInterrupt(1, lcd_timer_handler);
    Timer2.refresh();
    // Timer2.resume();
};

// set [row];[col];[inverse];[cursor]
//  [inverse], [cursor] = t or f
void on_cmd_set(char *param) {
    if (!param) { return; };
    char *s_row = strsep(&param, "/");
    char *s_col = strsep(&param, "/");
    char *s_inverse = strsep(&param, "/");
    char *s_cursor = param;
    int row = -1;
    int col = -1;
    if (s_row && s_row[0] != 0) {
        row = strtol(s_row, NULL, 10);
    }
    if (s_col && s_col[0] != 0) {
        col = strtol(s_col, NULL, 10);
    }
    if (col >= 0 && row >= 0) {
        scr_move_to(row, col);
    }
    if (s_inverse) {
        if (s_inverse[0] == 't') {
            screen_char_mode = screen_char_mode | MODE_INVERSE;
        } else if (s_inverse[0] == 'f') {
            screen_char_mode = screen_char_mode & (~MODE_INVERSE);
        }
    }
    if (s_cursor) {
        if (s_cursor[0] == 't') {
            cursor_flag = cursor_flag | CURSOR_ON;
        } else if (s_cursor[0] == 'f') {
            cursor_flag = cursor_flag & (~CURSOR_ON);
        }
    }
    
};

// text [settings] <text>
void on_cmd_text(char *param) {
    char *set = strsep(&param, ",");
    if (set && set[0] != 0) {
        on_cmd_set(set);
    };
    scr_print(param);
};

// backlight,on/off
void on_cmd_backlight(char *param) {
    if (strcmp("on", param) == 0) {
        lcd_backlight_on();
        backlight_time = BACKLIGHT_TIME;
    } else if (strcmp("off", param) == 0) {
        lcd_backlight_off();
    }
};


int on_cmd(char *buf) {
    if (!buf || buf[0] == 0) { return false;};
    char *cmd   = strsep(&buf, ",");
    char *param = buf;
    if (!cmd || cmd[0] == 0) { return false;};
    if (strcmp(cmd, "cursor") == 0) {
        if (strcmp(param, "on") == 0) {
            scr_cursor_on();
        } else if (strcmp(param, "off") == 0) {
            scr_cursor_off();
        } else {
            scr_println("cursor on|off");
        }
    } else if (strcmp(cmd, "clear") == 0) {
        if (connected) {
            scr_clean();
        } else {
            lcd_clear_graphic();
            lcd_text_init();
            lcd_clear_text();
            lcd_clear_attribute();
            lcd_move_to(0, 0);
            scr_clean();
            Timer2.resume();
            connected = true;
        }
    } else if (strcmp(cmd, "reset") == 0) {
            scr_clean();
            on_cmd_set(param);
    } else if (strcmp(cmd, "text") == 0) {
        on_cmd_text(param);
    } else if (strcmp(cmd, "backlight") == 0) {
        on_cmd_backlight(param);
    } else {
        // scr_print("unknown command:");
        // scr_println(cmd);
    }
    return 0;
}

/*
  按键值存入btn_queue_buf
*/
void btn_queue_in(Btn_code btn)
{
    //缓冲区满
    if (((btn_queue_buf_head + 1) % sizeof(btn_queue_buf)) == btn_queue_buf_tail) {
        return;
    }

    btn_queue_buf[btn_queue_buf_head] = btn;
    btn_queue_buf_head ++;
    btn_queue_buf_head = btn_queue_buf_head % sizeof(btn_queue_buf);
}

void on_btn_pressed(Btn_code btn) {

    backlight_time = BACKLIGHT_TIME;
    lcd_backlight_on();
    
    Serial2.print("btn-p,");
    switch(btn) {
    case BTN_NOP:   Serial2.println("nop");   break;
    case BTN_UP:    Serial2.println("up");    break;
    case BTN_DOWN:  Serial2.println("down");  break;
    case BTN_LEFT:  Serial2.println("left");  break;
    case BTN_RIGHT: Serial2.println("right"); break;
    case BTN_OK:    Serial2.println("ok");    break;
    case BTN_BACK:  Serial2.println("back");  break;
    }
}

void on_btn_release(Btn_code btn) {\
    Serial2.print("btn-r:");
    switch(btn) {
    case BTN_NOP:   Serial2.println("nop");   break;
    case BTN_UP:    Serial2.println("up");    break;
    case BTN_DOWN:  Serial2.println("down");  break;
    case BTN_LEFT:  Serial2.println("left");  break;
    case BTN_RIGHT: Serial2.println("right"); break;
    case BTN_OK:    Serial2.println("ok");    break;
    case BTN_BACK:  Serial2.println("back");  break;
    }
}

void check_btn() {
    byte mask = 0x01;
    for (byte i = 0; i < sizeof(btn_pin_bits); i++) {
        if (digitalRead(btn_pin_bits[i]) == LOW) {
            if (btn_pin_state & mask) {
            } else {
                btn_pin_state = btn_pin_state | mask;    // press
                btn_timer[i] = 30;
            }
        } else {
            if (btn_pin_state & mask) {
                btn_pin_state = btn_pin_state & (~mask); // release
                btn_timer[i] = -1;
                // Btn_code code = (Btn_code)i;
                // on_btn_release(code);
            } else {
            }
        }
        mask = mask << 1;
    };
};

#define CMD_BUF_LEN  512
char cmd_buf[CMD_BUF_LEN] = {0};
byte cmd_buf_ptr = 0;

unsigned long timer_btn_in_count  = micros();
unsigned long timer_btn_out_count = micros();
void tick_1ms() {
    for (byte i = 0; i < 6; i++) {
        if (btn_timer[i] > 0) {
            btn_timer[i]--;
            if (btn_timer[i] == 0) {
                btn_timer[i] = -1;
                Btn_code code = (Btn_code)i;
                //按键存入缓冲区
                btn_queue_in(code);
            }
        }
    }
};


void loop() {
    unsigned long t  = micros();
    unsigned long t1 = micros();
    if (t - timer_btn_in_count > 1000){
        tick_1ms();
        timer_btn_in_count = t;
    }
    check_btn();

    //缓冲区中有按键
    if (btn_queue_buf_head != btn_queue_buf_tail) {
        Btn_code btn = btn_queue_buf[btn_queue_buf_tail];
        on_btn_pressed(btn);
        btn_queue_buf_tail = (btn_queue_buf_tail + 1) % sizeof(btn_queue_buf);
    }

    if (Serial2.available()) {
        char ch = Serial2.read();
        if (ch == 0x0A || ch == 0x0D) {
            on_cmd(cmd_buf);
            cmd_buf_ptr = 0;
            memset(cmd_buf, 0, CMD_BUF_LEN);
        } else {
            if (cmd_buf_ptr < CMD_BUF_LEN) {
                cmd_buf[cmd_buf_ptr] = ch;
                cmd_buf_ptr++;
                cmd_buf[cmd_buf_ptr] = 0;
            }
        }
    }
}


__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
    setup();
    
    while (true) {
        loop();
    }
    return 0;
}

