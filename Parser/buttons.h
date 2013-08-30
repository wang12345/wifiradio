#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "cache.h"





typedef struct tagBUTTONS
{
	CACHE events;
	int posin;
	int posout;
	unsigned char buttons;
	int counter;
	unsigned long timer;
	unsigned long lasttimer;
} BUTTONS, *PBUTTONS;


typedef struct tagCONTROL
{
	int pos;
	int maxpos;
	int volume;
	unsigned char isstop;
	unsigned char isplay;
	unsigned char isready;
	unsigned long lasttimer;


} CONTROL,*PCONTROL;



void BUTTONS_Init(PBUTTONS b,PCONTROL c,char* pbuf,unsigned int bufsize);
void BUTTONS_Process();
void BUTTONS_SetMaxPos(int p);

#endif
