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

#include "eeprom.h"
/*


 25LC040  stellaris
 CS			D6      chip select
 SO         D2      read from the 25LC040
 WP         +5      write protect
 HOLD       +5
 SCK        D0      clock
 SI         D3      write to the 25LC040



 */

#define GPIO_PIN_CS GPIO_PIN_6


#define EEPROMADDRTEST    0
#define EEPROMADDRVOLUME  2
#define EEPROMADDRPOS     4

#define EEPROMADDRSTRMIN     6
#define EEPROMADDRSTRMAX     508
#define EEPROMADDRSTRCOUNT   50
#define EEPROMADDRSTRLEN     10

#define EEPROMADDRTESTPATTERN1    0xA5A5
#define EEPROMADDRTESTPATTERN2    0x5A5A
#define EEPROMNULL 0xFFFF

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0
#define NOTSET -1


// evil magic
#define DELAY() SysCtlDelay(50000)


#define CS_ON() {GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_CS,0x00);SysCtlDelay(100);}
#define CS_OFF() {GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_CS,0xFF);SysCtlDelay(100);}



unsigned char _iseepromenabled = FALSE;
char _eepromcachestr[EEPROMADDRSTRLEN+1];
int _eepromcacheindex = NOTSET;


void EEPROM_PutInt(int addr,short int value);
short int EEPROM_GetInt(int addr);


void PUT(unsigned char data)
{
	SSIDataPut(SSI1_BASE, data);
	while(SSIBusy(SSI1_BASE)) {};
}


unsigned char EEPROM_Test()
{

	if (_iseepromenabled==TRUE) return TRUE;
	_iseepromenabled=TRUE;
	EEPROM_PutInt(EEPROMADDRTEST ,EEPROMADDRTESTPATTERN1);
	if (EEPROM_GetInt(EEPROMADDRTEST)!=EEPROMADDRTESTPATTERN1) return FALSE;
	EEPROM_PutInt(EEPROMADDRTEST ,EEPROMADDRTESTPATTERN2);
	if (EEPROM_GetInt(EEPROMADDRTEST)!=EEPROMADDRTESTPATTERN2) return FALSE;
	return TRUE;
}


void EEPROM_Init()
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);



   GPIOPinConfigure(GPIO_PD0_SSI1CLK);
   GPIOPinConfigure(GPIO_PD1_SSI1FSS);
   GPIOPinConfigure(GPIO_PD2_SSI1RX);
   GPIOPinConfigure(GPIO_PD3_SSI1TX);
   GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_CS);


   GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2|GPIO_PIN_3);


   SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000, 8);

   SSIEnable(SSI1_BASE);
   
   memset(_eepromcachestr,0,EEPROMADDRSTRLEN+1);
   _iseepromenabled  = EEPROM_Test();

}




unsigned char EEPROM_IsActive()
{
	return  _iseepromenabled;
}




void EEPROM_Write(int addr,unsigned char* pdata,int len)
{

	int i;
	unsigned long tmp;
	volatile unsigned char c1;
	volatile unsigned char c2;
	volatile unsigned char c3;
	volatile unsigned char* pd = pdata;

	c1 = 0x02; //  Write command
	if (addr>0xFF) c1|=8; // address 9th bit
	c2 = addr;

	CS_ON();
    PUT(0x06);//// Send WREN (Enable Write Operations)
	CS_OFF();


	CS_ON();
	while( SSIDataGetNonBlocking(SSI1_BASE, &tmp)) {} ;
	while(SSIBusy(SSI1_BASE)) {};

	PUT(c1);
	PUT(c2);
	for(i=0;i<len;i++)
	{	c3 = *(pd+i);
	    PUT(c3);
	}
	CS_OFF();

	DELAY();

}

#define UNINITIALIZED 0xff
#define ZERO 0
void EEPROM_Read(int addr,unsigned char* pdata,int len,unsigned char ischar)
{
	int i=0;
	volatile unsigned long tmp=0;
	volatile unsigned char c1=0;
	volatile unsigned char c2=0;

	c1 = 0x03; // Read command
	if (addr>0xFF) c1|=8; // address 9th bit
	c2 = addr ;

	CS_ON();

	while(SSIBusy(SSI1_BASE)) {};

	PUT(c1);
	PUT(c2);
	while( SSIDataGetNonBlocking(SSI1_BASE,(unsigned long*) &tmp)) {} ;
	for(i=0;i<len;i++)
	{	PUT(0x00);//// Send dummy Byte command
		while(SSIDataGetNonBlocking(SSI1_BASE, (unsigned long*) &tmp))// Fetch data from RX buffer
		{
			DELAY();
		}
		if ((ischar) && (tmp==UNINITIALIZED)) tmp = ZERO;
		*(pdata+i) = tmp;
	}
	CS_OFF();

	DELAY();

}





short int EEPROM_GetInt(int addr)
{
	volatile int i;
	if (_iseepromenabled )
	{	EEPROM_Read(addr,(unsigned char*)&i,2,FALSE);
	} else
	{	i = EEPROMNULL;
	}
	return i;
}

void EEPROM_PutInt(int addr,short int value)
{
	volatile int i = value;
	if (_iseepromenabled )
		EEPROM_Write(addr,(unsigned char*)&i,2);
}



void EEPROM_PutIntEnsured(int addr,short int value)
{

	_iseepromenabled=TRUE;

	EEPROM_PutInt(addr ,value);

	if (EEPROM_GetInt(addr)!=value)
	{
		_iseepromenabled=FALSE;
	};

}



int EEPROM_GetVolume()
{
	//return  EEPROMNULL;
	return EEPROM_GetInt(EEPROMADDRVOLUME);
}

void EEPROM_PutVolume(int volume)
{
	EEPROM_PutIntEnsured(EEPROMADDRVOLUME,volume);
}


int EEPROM_GetPos()
{
	//return  EEPROMNULL;
	return EEPROM_GetInt(EEPROMADDRPOS);
}

void EEPROM_PutPos(int pos)
{
	EEPROM_PutIntEnsured(EEPROMADDRPOS,pos);
}


void EEPROM_PutCache(char* ptitle,int index,int maxlen)
{
	memcpy(_eepromcachestr,ptitle,EEPROMADDRSTRLEN);
	_eepromcacheindex = index;
}


unsigned char EEPROM_CheckCache(char* ptitle,int index,int maxlen)
{
	int i;
	int n = maxlen>EEPROMADDRSTRLEN  ? EEPROMADDRSTRLEN : maxlen;
	if (index!=_eepromcacheindex) return FALSE;
	for(i=0;i<n;i++)
	{	if (*(ptitle+i)==0) return TRUE;
		if (_eepromcachestr[i]!=*(ptitle+i)) return FALSE;
	}
	return TRUE;
}


void EEPROM_PutTitle(char* ptitle,int index,int maxlen)
{

	int n = maxlen>EEPROMADDRSTRLEN  ? EEPROMADDRSTRLEN : maxlen;
	int a;
	if (_iseepromenabled )
	{	a = (index*EEPROMADDRSTRLEN) + EEPROMADDRSTRMIN;
		if (a+n<EEPROMADDRSTRMAX)
		{	if (EEPROM_CheckCache(ptitle,index,maxlen)==FALSE)
			{	EEPROM_Write(a,(unsigned char*)ptitle,n);
				EEPROM_PutCache(ptitle,index,maxlen);
			}
		}
	} 

}

void EEPROM_GetTitle(char* ptitle,int index,int maxlen)
{
    int n = maxlen>EEPROMADDRSTRLEN  ? EEPROMADDRSTRLEN : maxlen;
	int a;
	*(ptitle) = 0;

	if (_iseepromenabled )
	{	a = (index*EEPROMADDRSTRLEN) + EEPROMADDRSTRMIN;
		if (a+n<EEPROMADDRSTRMAX)
		{	EEPROM_Read(a,(unsigned char*)ptitle,n,TRUE);
			*(ptitle+n) = 0;
		}
	} 

}

