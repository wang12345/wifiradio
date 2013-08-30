#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/timer.h"


#include "buttons.h"
#include <string.h>
#include "types.h"

PBUTTONS _pbuttons;
PCONTROL _pctrl;



#define TIMER_INTERVAL 10000

#define TIMER_SKIP_INTERVAL 1000

void PortBIntHandler()
{
	unsigned char port;
	unsigned char b=0x80;
	unsigned char perv,curr;
	int i;

	port=0;


	for(i=7;i>=0;i--)
	{
		perv = _pbuttons->buttons & b ? TRUE : FALSE;
		curr = ROM_GPIOPinRead(GPIO_PORTB_BASE,b) ? TRUE : FALSE;
		if (curr) port|=b;
		if (curr!=perv)
		{
			if (curr) // release
			{
			} else  //press
			{	if (DIFF(_pbuttons->lasttimer,_pbuttons->timer)>TIMER_SKIP_INTERVAL)
				{
					CACHE_Put(&(_pbuttons->events),i);
					_pbuttons->lasttimer = _pbuttons->timer;
				}
			}
		}
		b = b >> 1;
	}

	_pbuttons->buttons =port;

	ROM_GPIOPinIntClear(GPIO_PORTB_BASE, 0xFF );
}

void TimerHandler()
{
	ROM_TimerIntClear( TIMER0_BASE, TIMER_TIMA_TIMEOUT ) ;
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, TIMER_INTERVAL);

	_pbuttons->timer++;
}

void BUTTONS_Init(PBUTTONS b,PCONTROL c,char* pbuf,unsigned int bufsize)
{
	memset(b,0,sizeof(BUTTONS));
	_pbuttons = b;
	_pbuttons->buttons=0xFF;

	_pctrl = c;

	CACHE_Init(&(_pbuttons->events),pbuf,bufsize);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, 0xFF);
    ROM_GPIOPadConfigSet(GPIO_PORTB_BASE, 0xFF, GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

    GPIOPortIntRegister(GPIO_PORTB_BASE, PortBIntHandler);
    ROM_GPIOIntTypeSet(GPIO_PORTB_BASE, 0xFF ,GPIO_BOTH_EDGES);
    ROM_GPIOPinIntEnable(GPIO_PORTB_BASE, 0xFF);
    ROM_IntEnable(INT_GPIOB);


	ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 ) ;
	ROM_TimerConfigure( TIMER0_BASE, TIMER_CFG_PERIODIC ) ;
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, TIMER_INTERVAL);
	TimerIntRegister( TIMER0_BASE, TIMER_A,TimerHandler ) ;
	ROM_TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT ) ;
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerEnable( TIMER0_BASE, TIMER_A ) ;

}


void BUTTONS_SetMaxPos(int p)
{
	_pctrl->maxpos = p;

}


void BUTTONS_Process()
{





}



