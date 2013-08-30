#include "softSPI_9bit.h"
#include "inc/lm4f120h5qr.h"

/*

# oled stellaris

1  3.3
2  GND
3  d7
4  d6
5  d5
6  d4
7  d3
8  d2
9  d1  PA5 - TX
10 d0  PA2 - CLK
11 RD
12 RW
13 DC
14 RES PA6 - reset
15 CS  PA7 - CS
16

*/


#define PIN_SCLK  BIT2
#define PIN_MOSI  BIT5
#define PIN_CS    BIT7
#define PIN_POWER BIT6


#define POWER_UP() {GPIO_PORTA_DATA_R|=PIN_POWER;}
#define POWER_DN() {GPIO_PORTA_DATA_R&=~PIN_POWER;}

#define SET_CS()    {GPIO_PORTA_DATA_R |=  PIN_CS;}
#define CLR_CS()    {GPIO_PORTA_DATA_R &= ~PIN_CS;}
#define SET_MOSI()   {GPIO_PORTA_DATA_R |=  PIN_MOSI;}
#define CLR_MOSI()   {GPIO_PORTA_DATA_R &= ~PIN_MOSI;}
#define SET_SCK()    {GPIO_PORTA_DATA_R |=  PIN_SCLK;}
#define CLR_SCK()    {GPIO_PORTA_DATA_R &= ~PIN_SCLK;}


void SPI9_Delay()
{
	//volatile int i;
	//for(i=0;i<5;i++);;

}

void SPI9_DelayLong()
{
	volatile int i;
	for(i=0;i<10000;i++);

}




void SPI9_Init()
{

	 SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

	 SPI9_Delay();


	 GPIO_PORTA_DIR_R |= PIN_SCLK  | PIN_MOSI | PIN_CS |  PIN_POWER;
	 GPIO_PORTA_DEN_R |= PIN_SCLK  | PIN_MOSI | PIN_CS |  PIN_POWER;


	 POWER_DN();
	 SPI9_DelayLong();
	 POWER_UP();
	 SPI9_DelayLong();


}

void SPI9_Finit()
{
	GPIO_PORTA_DATA_R &= ~(PIN_SCLK  | PIN_MOSI | PIN_CS |  PIN_POWER);
	POWER_DN();
}




void SPI9_Write( UINT16 tx )
{
    UINT16 i = 0x100;
    volatile UINT8 p1in;


    CLR_CS();
    SPI9_Delay();
    while( i )
    {
        if ( i & tx )  	SET_MOSI() 	else  CLR_MOSI();
        SET_SCK();
        SPI9_Delay();
        CLR_SCK();
        SPI9_Delay();
        i >>= 1;
    }
    SET_CS();
    SPI9_Delay();

}

