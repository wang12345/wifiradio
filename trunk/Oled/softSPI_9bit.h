// This is 9bit write only SPI protocol for OLED SSD1306


#ifndef _SPI9_H
#define _SPI9_H



#define BIT0                   (0x0001)
#define BIT1                   (0x0002)
#define BIT2                   (0x0004)
#define BIT3                   (0x0008)
#define BIT4                   (0x0010)
#define BIT5                   (0x0020)
#define BIT6                   (0x0040)
#define BIT7                   (0x0080)
#define BIT8                   (0x0100)
#define BIT9                   (0x0200)
#define BITA                   (0x0400)
#define BITB                   (0x0800)
#define BITC                   (0x1000)
#define BITD                   (0x2000)
#define BITE                   (0x4000)
#define BITF                   (0x8000)




#define UINT16 unsigned int
#define UINT8 unsigned char
#define TRUE 1
#define FALSE 0
#define BOOL UINT8
#define ON 1
#define OFF 0




void SPI9_Init();
void SPI9_Finit();
void SPI9_Write( UINT16 tx );

#endif

