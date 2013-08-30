#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include <string.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "Oled/OLED_SSD1306.h"
#include "Oled/Oled_Printf.h"
#include "Oled/Oled_Config.h"



#include "Parser/cache.h"
#include "Parser/uart_parser.h"
#include "Parser/buttons.h"
#include "Parser/types.h"
#include "utils/settings.h"
#include "utils/eeprom.h"
#include "utils/max7219.h"


//#define DUMBMODE

#define VERSION "ver. 2.1  16.08.13"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

#define BUTTON_1 GPIO_PIN_0
#define BUTTON_2 GPIO_PIN_4

//echo "currentsong" | nc localhost 6600
//echo "status" | nc localhost 6600

#define CMD_LOAD "~/.mpd/mpc_update.sh\n"
#define CMD_STOP "mpc stop\n"
#define CMD_NEXT "mpc next\n"
#define CMD_PREV "mpc prev\n"
#define CMD_PLAY "mpc play\n"
#define CMD_SHOW "~/.mpd/mpc_show.sh\n"
#define CMD_VOLUME_FORMAT "mpc volume ######"
#define CMD_VOLUME_FORMAT_POS 11
#define CMD_PLAY_FORMAT "mpc play ######"
#define CMD_PLAY_FORMAT_POS 9



//  RADIO #1
#define BUTTON_PLAYSTOP 3
#define BUTTON_NEXT     0
#define BUTTON_VOLUP    4
#define BUTTON_VOLDN    1
#define BUTTON_PREV     2


/*
//  RADIO #0
#define BUTTON_PLAYSTOP 0
#define BUTTON_NEXT     2//6
#define BUTTON_VOLUP    4
#define BUTTON_VOLDN    5
#define BUTTON_PREV     3
*/


#define VOLUME_DEFAULT 5

#define VOLUME_MAX 100
#define VOLUME_MIN 0
#define VOLUME_STEP 1
#define VARCMDMAX 20

#define UARTCACHEBUFFERSIZE 1024
#define BUTTONSCACHESIZE 25

#define LINUXBOOTTIMERMAX 200000


#define CHANNEL_SELECT_TIMEOUT 1500 //in main cycle counts
#define UARTACTIVITYSHOWTIMEOUT 6000
#define SCROLLTIMEOUT 600
#define SEND_STATUS_CMD_INTERVAL 500000
#define SEND_STATUS_CMD_DELAY 5000
#define LEDANIMATIONTIMEOUT 800

#define UARTACTIVITYSHOWNONE (0xFFFFFFFF-UARTACTIVITYSHOWTIMEOUT-1)



char _uartcachebuffer[UARTCACHEBUFFERSIZE];
char _buttonscache[BUTTONSCACHESIZE];
unsigned long _u0rxtime,_u0txtime;
UARTSTATUS _status;
CACHE _cache;
BUTTONS _buttons;
CONTROL _ctrl;
char _UARTACTIVITY[4] = {CHAR_UART_NONE,CHAR_UART_IN,CHAR_UART_OUT,CHAR_UART_INOUT};
unsigned long _scrolltimer;
unsigned long _statuscmdtimer;

unsigned long _chseltimer;
BOOL _ischselectmode;
int _channeltoset;
int _ledanimationcounter;
unsigned long _ledanimationtimer;
unsigned long _linuxboottimer;



#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif



void ScheduleShowCommand();
void WaitASecond();
void SendCommand(char* cmd);
void C_CommandString(char* c,int pos,int value);
void C_PlayFromPos(int value);




char GetUartActivityChar()
{
	unsigned char n=0;
	if (DIFF(_buttons.timer,_u0rxtime) < UARTACTIVITYSHOWTIMEOUT) n|=1;
	if (DIFF(_buttons.timer,_u0txtime) < UARTACTIVITYSHOWTIMEOUT) n|=2;
	return _UARTACTIVITY[n];

}

void UartActivityDetected(BOOL isrx,BOOL istx)
{
	if (isrx) _u0rxtime =  _buttons.timer;
	if (istx) _u0txtime =  _buttons.timer;
}


void UART1IntHandler(void)
{
    unsigned long ulStatus;
    unsigned char uc;


    ulStatus = ROM_UARTIntStatus(UART1_BASE, true);

    ROM_UARTIntClear(UART1_BASE, ulStatus);

    while(ROM_UARTCharsAvail(UART1_BASE))
    {
    	uc =  ROM_UARTCharGetNonBlocking(UART1_BASE);
    	ROM_UARTCharPutNonBlocking(UART0_BASE, uc);
		CACHE_Put(&_cache,uc);
		UartActivityDetected(TRUE,FALSE);
    }
}



void UART0IntHandler(void)
{
    unsigned long ulStatus;
    unsigned char uc;


    ulStatus = ROM_UARTIntStatus(UART0_BASE, true);

    ROM_UARTIntClear(UART0_BASE, ulStatus);

    while(ROM_UARTCharsAvail(UART0_BASE))
    {
    	uc =  ROM_UARTCharGetNonBlocking(UART0_BASE);
    	ROM_UARTCharPutNonBlocking(UART1_BASE, uc);
    	UartActivityDetected(FALSE,TRUE);
    }
}





void ScheduleShowCommand()
{
	_statuscmdtimer = _buttons.timer - (SEND_STATUS_CMD_INTERVAL - SEND_STATUS_CMD_DELAY);
}

void WaitASecond()
{
	SysCtlDelay(1000000);
}

void SendCommand(char* cmd)
{
	unsigned int i,n;

	n = strlen(cmd);
	for(i=0;i<n;i++)
	{
		ROM_UARTCharPut(UART1_BASE, *(cmd+i));
		UartActivityDetected(FALSE,TRUE);
		SysCtlDelay(1000);
	}
}


void C_CommandString(char* c,int pos,int value)
{
	unsigned char hundreds,tens,ones;
	unsigned char n;
	char cmd[VARCMDMAX];
    strcpy(cmd,c);
    hundreds = value / 100;
    tens = (value-hundreds*100) / 10;
    ones = value-hundreds*100 - tens*10;
    n=pos;
    if (hundreds!=0)
    {	cmd[n++] = '0'+hundreds;
    }
    if ((tens!=0)||(hundreds!=0))
    {	cmd[n++] = '0'+tens;
    }
    cmd[n++] = '0'+ones;
    cmd[n++] = '\n';
    cmd[n++] = 0;
    SendCommand(cmd);
    ScheduleShowCommand();

}

void UpdateScreen()
{
	UP_ClearTraps(UP_CLEAR_SCREEN);
	SendCommand(CMD_SHOW);
}


void C_Command(char* cmd)
{
	UP_ClearTraps(UP_CLEAR_SCREEN);
	SendCommand(cmd);
	ScheduleShowCommand();
}

void C_Update()
{	UP_ClearTraps(UP_CLEAR_SCREEN);
	SendCommand(CMD_LOAD);

}


void SetDirtyFlag()
{
	_status.IsDirty = TRUE;
}


void CheckMoveChannel()
{
	if (_ischselectmode==FALSE) return;

	_chseltimer--;

	if (_chseltimer<=0)
	{	C_PlayFromPos(_channeltoset);
		_ischselectmode=FALSE;
		SetDirtyFlag();
	}

}


void C_MoveChannel(int N)
{
	if ((_status.Pos == NOTSET)||(_status.MaxPos == NOTSET))
	{
		if (N==1)
		{	C_Command(CMD_NEXT);
			return;
		}
		if (N==-1)
		{	C_Command(CMD_PREV);
			return;
		}
		return;
	}


	if (_ischselectmode==FALSE)
	{	_ischselectmode=TRUE;
		_channeltoset = _status.Pos+1;
	}

	_channeltoset+=N;
	if (_channeltoset<1) _channeltoset = _status.MaxPos +  _channeltoset;
	if (_channeltoset>_status.MaxPos) _channeltoset = _channeltoset - _status.MaxPos;
	_chseltimer = CHANNEL_SELECT_TIMEOUT;
	SetDirtyFlag();


}

void C_Play()
{	C_Command(CMD_PLAY);
}

void C_Stop()
{	C_Command(CMD_STOP);
}


void C_PlayFromPos(int value)
{
	UP_ClearTraps(UP_CLEAR_SCREEN);
	C_CommandString(CMD_PLAY_FORMAT,CMD_PLAY_FORMAT_POS,value);
	EEPROM_PutPos(value);

}

void C_Volume(int value)
{
	C_CommandString(CMD_VOLUME_FORMAT,CMD_VOLUME_FORMAT_POS,value);
	EEPROM_PutVolume(value);
}

void OledPrint(int N)
{
	if (N>999)
	{
		OLED_Printf("%s","###");
		return;
	};
	if (N==NOTSET)
	{	OLED_Printf("%s","---");
		return;
	}

	if (N<10)  OLED_Printf("0");
	if (N<100) OLED_Printf("0");
	OLED_Printf("%d",N);


}
#define OLED_SMALLFONTWIDTH 25
#define SCROLL_TAIL 10
void OledScroll(int line,char* str,int scroll)
{

	int n,pos,i;
	char c;
	OLED_PrintfSetColumnRow(0,line,SMALL_FONT);

	n = strlen(str);
	if (n<=OLED_SMALLFONTWIDTH)
	{	if (n>0) n--;
		for(i=0;i<OLED_SMALLFONTWIDTH;i++)
			OLED_Printf("%c",(i<n) ? *(str+i) : ' ');
		return;
	}
	pos = scroll % (n+SCROLL_TAIL);
	for(i=0;i<OLED_SMALLFONTWIDTH;i++)
	{
		if (pos>=n)
		{	c = ' ';
		} else
		{	c = *(str+pos);
		    if ((c>0x81)||(c<32)) c=' ';
		}
		OLED_Printf("%c",c);
		pos++;
		if (pos>n+SCROLL_TAIL) pos=0;

	}

}


void OledPrintBin(unsigned char n)
{
	unsigned char b;
	int i;

	b=0x80;
	for(i=0;i<8;i++)
	{
		OLED_Printf("%c",(n & b) ? '1' : '0');
		b = b >> 1;
	}
}

#define PROGRESSSTRLEN 9
void OledPrintPersent(int persent)
{
	int n;
	int i;
	char s[PROGRESSSTRLEN+1];
	memset(s,0,PROGRESSSTRLEN+1);

	n = (persent*PROGRESSSTRLEN*2)/100;

	for(i=0;i<PROGRESSSTRLEN;i++)
	{
		if ((i*2<n) && (i*2+1<n))
		{	s[i]=CHAR_PROGRESS_FULL;
			continue;

		}
		if ((i*2<n) && (i*2+1>=n))
		{	s[i]=CHAR_PROGRESS_HALF;
			continue;
		}
		s[i]=CHAR_PROGRESS_NULL;

	}

	OLED_PrintfSetColumnRow(0,7,LARGE_FONT);
	OLED_Printf(s);
}




void PrintWIFIRadio()
{


	OLED_PrintfSetColumnRow(0,7,LARGE_FONT);
	OLED_Printf(" WI-FI");
	OLED_PrintfSetColumnRow(0,4,LARGE_FONT);
	OLED_Printf(" RADIO");
	OLED_PrintfSetColumnRow(0,1,SMALL_FONT);
	OLED_Printf(VERSION);
}

void PrintStatus(char* text)
{
	OLED_PrintfSetColumnRow(0,0,SMALL_FONT);
	OLED_Printf(text);

}


void ProcessStatus()
{

	volatile int tmp;

	if (_status.Status == STATUS_BOOTDONE)
	{
		_status.Status = STATUS_LOADING;
		SendCommand("\n\n\n");
		WaitASecond();
		C_Update();
		WaitASecond();
		return;

	}

	if (_status.Status == STATUS_READY)
	{
		EEPROM_Test();

		tmp = EEPROM_GetVolume();
		if ((tmp>VOLUME_MAX)||(tmp<VOLUME_MIN)) tmp = VOLUME_DEFAULT;
		C_Volume(tmp);
		WaitASecond();
		tmp = EEPROM_GetPos();
		C_PlayFromPos(tmp);
		WaitASecond();

		//magic
		MAX7219Init();


		_status.Status = STATUS_WORKING;

	}


	if (_status.Status == STATUS_BOOTING)
	{
		_linuxboottimer = _buttons.timer;

	}

}




#define NAMETMPSTRLEN 25
void FillOLED()
{
	char namestr[NAMETMPSTRLEN];




	if (_ischselectmode==TRUE)
	{
		OLED_PrintfSetColumnRow(40,4,LARGE_FONT);
		OledPrint(_channeltoset);
		MAX7219PrintInt(_channeltoset);
		EEPROM_GetTitle(namestr,_channeltoset-1,NAMETMPSTRLEN);
		PrintStatus(namestr);
		return;
	}



	if (_status.Status == STATUS_UNKNOWN)
	{
		MAX7219Clear();
		if (_status.PlayStatus == PLAY_YES)
		{	_status.Status = STATUS_WORKING;
		} else
		{	PrintWIFIRadio();
			if (EEPROM_IsActive()==FALSE)
				PrintStatus("EEPROM fail...");
			return;
		}
	}


	if (_status.Status == STATUS_BOOTING)
	{
		MAX7219Clear();
		PrintWIFIRadio();
		PrintStatus("Loading Linux...");
		WaitASecond();
		return;

	}

	if (_status.Status == STATUS_BOOTDONE)
	{
		MAX7219Clear();
		PrintWIFIRadio();
		PrintStatus("Linux loaded");
		return;

	}

	if (_status.Status == STATUS_LOADING)
	{
		MAX7219Clear();
		PrintWIFIRadio();
		PrintStatus("Updating playlist");
		return;

	}

	if (_status.Status == STATUS_READY)
	{
		MAX7219Clear();
		PrintWIFIRadio();
		PrintStatus("Almost done");
		return;
	}

	if (_status.Status == STATUS_ERROR)
	{
		MAX7219Clear();
		OLED_PrintfSetColumnRow(0,4,LARGE_FONT);
		OLED_Printf(" ERROR");
		PrintStatus("No connection");
		return;
	}


	if (_status.BitRate !=  BITRATE_UNKNOWN)
	{
		OLED_PrintfSetColumnRow(90,4,SMALL_FONT);
		OLED_Printf("%d",_status.BitRate);
	}



	BUTTONS_SetMaxPos(_status.MaxPos) ;


	OLED_PrintfSetColumnRow(0,7,LARGE_FONT);
	if (_status.Pos == NOTSET)
	{   MAX7219Waiting();
		OLED_Printf("---");
	} else
	{	MAX7219PrintInt(_status.Pos+1);
		OledPrint(_status.Pos+1);
	}
	OLED_Printf("/");
	if (_status.MaxPos == NOTSET)
	{   OLED_Printf("---");
	} else
	{	OledPrint(_status.MaxPos);
	}

	if (EEPROM_IsActive()==FALSE)
	{
		OLED_PrintfSetColumnRow(80,4,SMALL_FONT);
		OLED_Printf("E");
	}

	OLED_PrintfSetColumnRow(0,4,LARGE_FONT);
	OLED_Printf("%c",CHAR_SPEAKERON);
	OLED_PrintfSetColumnRow(20,4,LARGE_FONT);
	OledPrint(_status.Volume);
}




int main(void)
 {

	volatile unsigned long tmplong;
	volatile int tmp;

	volatile unsigned char b;
	volatile unsigned char f;

	_ledanimationcounter = 0;
	_ledanimationtimer =0;

	_u0rxtime=UARTACTIVITYSHOWNONE;
	_u0txtime=UARTACTIVITYSHOWNONE;

	_scrolltimer=0;
	_statuscmdtimer=0;

	_chseltimer=0;
	_ischselectmode=FALSE;
	_channeltoset=0;




	//ROM_FPUEnable();
    ROM_FPULazyStackingEnable();


    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

                       
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED | LED_GREEN | LED_BLUE);


    MAX7219PreInit();

    // UART1 (PC4,PC5) enable
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    ROM_GPIOPinConfigure(GPIO_PC4_U1RX);
    ROM_GPIOPinConfigure(GPIO_PC5_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
   // ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTStdioInit(1);
    UARTIntRegister(UART1_BASE,UART1IntHandler);



    // UART0 (PA0,PA1) enable
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
   // ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTStdioInit(0);
    UARTIntRegister(UART0_BASE,UART0IntHandler);

    // magic
    SysCtlDelay(100000);

    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    ROM_IntEnable(INT_UART1);
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

    
    SSD1306PinSetup();
    SSD1306Init();

    EEPROM_Init();



	UP_CreateTraps(&_status);
	UP_ClearTraps(UP_CLEAR_ALL);	
	CACHE_Init(&_cache,(char*)&_uartcachebuffer,UARTCACHEBUFFERSIZE);
	BUTTONS_Init(&_buttons,&_ctrl,(char*)&_buttonscache,BUTTONSCACHESIZE);
	

	ROM_IntMasterEnable();


	MAX7219Init();

	// magic, cant be done just after init
	EEPROM_Test();


    while(1)
    {



    	while ( !CACHE_IsEmpty(&(_buttons.events)))
    	{
    		b = CACHE_Get(&(_buttons.events));


    		switch (b)
    		{
    			case BUTTON_PLAYSTOP:
    				if (_status.PlayStatus == PLAY_YES)
    				{	C_Stop();
    				} else
    				{	C_Play();
    				}
    				break;
    			case BUTTON_NEXT:
    				C_MoveChannel(1);
    				break;
    			case BUTTON_PREV:
    				C_MoveChannel(-1);
    				break;
    			case BUTTON_VOLUP:
    				if (_status.Volume!=NOTSET)
    				{	_status.Volume+=VOLUME_STEP;
    					if (_status.Volume>VOLUME_MAX) _status.Volume=VOLUME_MAX;
    					C_Volume(_status.Volume);
    				}
    				break;
    			case BUTTON_VOLDN:
    				if (_status.Volume!=NOTSET)
    				{	_status.Volume-=VOLUME_STEP;
    					if (_status.Volume< VOLUME_MIN) _status.Volume=VOLUME_MIN;
    				}
    				C_Volume(_status.Volume);
    				break;

    		}


    	}



    	/*
        GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN, light);

*/

#ifndef DUMBMODE
		while (!CACHE_IsEmpty(&_cache))
			UP_ProcessChar(CACHE_Get(&_cache));
#endif


        if (_status.IsDirty)
        {
         	SSD1306Clear();

         	ProcessStatus();

         	FillOLED();

         	UP_ClearDirty();
			
        }

		if ((_status.IsNameDirty)&&(_status.Pos!=NOTSET))
		{
			EEPROM_PutTitle(_status.Name,_status.Pos,UARTSTATUSSTRMAX);
			_status.IsNameDirty=FALSE;
		}

        if (_ischselectmode==FALSE)
        {
			if (_status.PlayStatus == PLAY_YES)
			{	OledScroll(0,_status.Title,_status.ScrollTitle);
				OledScroll(1,_status.Name,_status.ScrollName);

			}

			OLED_PrintfSetColumnRow(114,4,LARGE_FONT);
			OLED_Printf("%c",(_status.PlayStatus == PLAY_YES) ? CHAR_PLAY : ((_status.PlayStatus == PLAY_NO) ? CHAR_STOP : ' '));

			OLED_PrintfSetColumnRow(114,7,LARGE_FONT);
			OLED_Printf("%c",GetUartActivityChar());

			if (DIFF(_buttons.timer,_scrolltimer) > SCROLLTIMEOUT)
			{	_scrolltimer=_buttons.timer;
				_status.ScrollName++;
				_status.ScrollTitle++;
			}
        }

      	CheckMoveChannel();

		//OLED_PrintfSetColumnRow(63,4,SMALL_FONT);
		//OLED_Printf("%d",_status.ScrollTitle);
		//OledPrintBin(_buttons.buttons);


        if ((GetUartActivityChar() == CHAR_UART_NONE) && (DIFF(_buttons.timer,_statuscmdtimer)>SEND_STATUS_CMD_INTERVAL))
        {
#ifndef DUMBMODE
        	if (_status.PlayStatus != PLAY_NO)
        		SendCommand(CMD_SHOW);
#endif
        	_statuscmdtimer = _buttons.timer;
        }



		//OLED_PrintfSetColumnRow(0,0,SMALL_FONT);
		//OLED_Printf("%d",_buttons.timer);
/*
		OLED_PrintfSetColumnRow(0,2,SMALL_FONT);
		OLED_Printf("%d",_buttons.counter);

*/


        if ((_status.Status == STATUS_UNKNOWN)||(_status.Status == STATUS_LOADING))
        {
			if (DIFF(_buttons.timer,_ledanimationtimer) > LEDANIMATIONTIMEOUT)
			{	_ledanimationtimer=_buttons.timer;
				MAX7219Animation(_ledanimationcounter++);
			}
        }

        if (_status.Status == STATUS_BOOTING)
	   {
			if (DIFF(_buttons.timer,_ledanimationtimer) > LEDANIMATIONTIMEOUT)
			{	_ledanimationtimer=_buttons.timer;

				 //magic
				 tmplong =  (((_linuxboottimer + LINUXBOOTTIMERMAX - _buttons.timer)*100)/LINUXBOOTTIMERMAX);
				 
				 MAX7219PrintTimer( tmplong );
			}
	   }


		if (_ischselectmode==TRUE)
		{
			OledPrintPersent((_chseltimer*100)/CHANNEL_SELECT_TIMEOUT);
		}






    	SysCtlDelay(10000);





    }
}




