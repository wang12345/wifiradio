#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include <string.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/ssi.h"
#include "max7219.h"
/*
 max7219   stellaris

 LOAD(CS)   e1      load pin
 SCK        e2      clock
 SI         e3      write to the max7219
 */


#define nop    0x00
#define digit0 0x01
#define digit1 0x02
#define digit2 0x03
#define digit3 0x04
#define digit4 0x05
#define digit5 0x06
#define digit6 0x07
#define digit7 0x08
#define decode 0x09
#define intensity 0x0A
#define scanlimit 0x0B
#define shutdown  0x0C
#define test 0x0F
#define EMPTY 0x0F
#define DASH 0x0A

int  portBase =   GPIO_PORTE_BASE;
#define CS GPIO_PIN_1
#define CLK  GPIO_PIN_2 // aka SCK
#define DIN   GPIO_PIN_3 //aka SI


#define DELAY() SysCtlDelay(10)


void writeBit(char c)
{
	GPIOPinWrite(portBase, CLK|DIN, 0	 |(DIN*c));
	DELAY();
	GPIOPinWrite(portBase, CLK|DIN, CLK|(DIN*c));
	DELAY();
}

void writeByte(unsigned char addr, unsigned char data){
	int i;
	GPIOPinWrite(portBase, CS|DIN|CLK, 0);
	DELAY();
	for(i=7;i>=0;i--){
		writeBit((addr&(1<<i))>>i);
	}
	for(i=7;i>=0;i--){
		writeBit((data&(1<<i))>>i);
	}
	GPIOPinWrite(portBase, CS|DIN|CLK, CS);
	DELAY();
}

void write2Byte(unsigned char addr, unsigned char data, unsigned char data2){
	int i;
	GPIOPinWrite(portBase, CS|DIN|CLK, 0);
	for(i=7;i>=0;i--){
		writeBit((addr&(1<<i))>>i);
	}
	for(i=7;i>=0;i--){
		writeBit((data&(1<<i))>>i);
	}
	for(i=7;i>=0;i--){
		writeBit((addr&(1<<i))>>i);
	}
	for(i=7;i>=0;i--){
		writeBit((data2&(1<<i))>>i);
	}
	GPIOPinWrite(portBase, CS|DIN|CLK, CS);
}






void MAX7219PreInit()
{

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(portBase,CS|CLK|DIN);
	GPIOPinWrite(portBase,CS|CLK|DIN,0x00);

}

void MAX7219Init()
{


	writeByte(shutdown,0x01);
	writeByte(test,0x00);
	writeByte(scanlimit,0x05);
	writeByte(intensity,0xF);
	writeByte(decode,0xFF);

	MAX7219Clear();

}



void MAX7219PrintInt(int n)
{
	if (n<10)
	{
		writeByte(digit0, EMPTY);
		writeByte(digit1, n);
		writeByte(digit2,EMPTY);
		writeByte(digit5, EMPTY);
		return;
	}

	if (n<100)
	{
		writeByte(digit0, EMPTY);
		writeByte(digit1,n % 10);
		writeByte(digit2,n / 10);
		writeByte(digit5, EMPTY);
		return;
	}
	if (n<1000)
	{
		writeByte(digit0,n % 10);
		writeByte(digit1,(n % 100) /10);
		writeByte(digit2,(n % 1000) / 100);
		writeByte(digit5,EMPTY);
		return;
	}
	if (n>9999)
	{
		writeByte(digit0,DASH);
		writeByte(digit1,DASH);
		writeByte(digit2,DASH);
		writeByte(digit5,DASH);
		return;
	}
	writeByte(digit0, n % 10);
	writeByte(digit1,(n % 100) /10);
	writeByte(digit2,(n % 1000) / 100);
	writeByte(digit5, (n % 10000) / 1000);

}


void MAX7219PrintTimer(unsigned long n)
{
	if ((n>100)||(n==0))
	{
		writeByte(digit0,DASH);
		writeByte(digit1,DASH);
		writeByte(digit2,DASH);
		writeByte(digit5,DASH);
		return;

	}
	writeByte(digit0, DASH);
	writeByte(digit1,n % 10);
	writeByte(digit2,n / 10);
	writeByte(digit5, DASH);

}


void MAX7219Animation(int n)
{
	switch(n%4)
	{
		case 3:
			writeByte(digit0,DASH);
			writeByte(digit1,EMPTY);
			writeByte(digit2,EMPTY);
			writeByte(digit5,EMPTY);
			break;
		case 2:
			writeByte(digit0,EMPTY);
			writeByte(digit1,DASH);
			writeByte(digit2,EMPTY);
			writeByte(digit5,EMPTY);
			break;
		case 1:
			writeByte(digit0,EMPTY);
			writeByte(digit1,EMPTY);
			writeByte(digit2,DASH);
			writeByte(digit5,EMPTY);
			break;
		case 0:
			writeByte(digit0,EMPTY);
			writeByte(digit1,EMPTY);
			writeByte(digit2,EMPTY);
			writeByte(digit5,DASH);
			break;
	}

}

void MAX7219Clear()
{
	writeByte(digit0,EMPTY);
	writeByte(digit1,EMPTY);
	writeByte(digit2,EMPTY);
	writeByte(digit5,EMPTY);
}

void MAX7219Waiting()
{
	writeByte(digit0,EMPTY);
	writeByte(digit1,DASH);
	writeByte(digit2,DASH);
	writeByte(digit5,EMPTY);
}
