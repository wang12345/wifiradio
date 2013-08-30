#ifndef OLED_CONFIG_H_
#define OLED_CONFIG_H_

enum e_columnIncrementFlag { NO_FONT, SMALL_FONT, LARGE_FONT };

#define LARGE_FONT_WIDTH	14
#define LARGE_FONT_HEIGHT	15
#define LARGE_FONT_SPAN		(LARGE_FONT_HEIGHT/8)+1

#define SMALL_FONT_WIDTH	5

#define CHAR_SPEAKERON  0x7B
#define CHAR_SPEAKEROFF 0x7C

#define CHAR_UART_NONE   0x7D
#define CHAR_UART_IN     0x7E
#define CHAR_UART_INOUT  0x7F
#define CHAR_UART_OUT    0x80
#define CHAR_PLAY        0x81
#define CHAR_STOP        0x82
#define CHAR_PAUSE       0x83
#define CHAR_PROGRESS_FULL      0x84
#define CHAR_PROGRESS_HALF      0x85
#define CHAR_PROGRESS_NULL      32

#endif /*OLED_CONFIG_H_*/
