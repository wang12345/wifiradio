/*
 * Copyright (c) 2011-2014 Mindspeed Technologies, Inc.. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify.
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but.
 * WITHOUT ANY WARRANTY; without even the implied warranty of.
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU.
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program; if not, write to the Free Software.
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution.
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Mindspeed Technologies
 */

#include "tiny_regex.h"
#include <string.h>

#define ALP   '?'  // one or more not space chars. see IsSpc function
#define SPC   '_'  // one or more space chars. see IsSpc function
#define NUM   '&'  // one or more numbers. see IsNum function
#define TXT   '@'  // one or more not numbers and not line feeds 
#define ANY   '*'  // one or more not line feed chars
#define NEW   '|'  // one or more line feed chars. see IsLineFeed function


#define REGEXTRUE 1
#define REGEXFALSE 0
#define REGEXNONE 0xFF

#define DONTRUNITAGAIN 0
#define RUNITONCEMORE   1

unsigned char IsLineFeed(char c);
unsigned char IsNum(char c);
unsigned char IsSpc(char c);
void Move(TINYRXINFO* pregex,unsigned char ismovetoanothermatch);
void Stop(TINYRXINFO* pregex,unsigned char result);
void CharFound(TINYRXINFO* pregex,char c,unsigned char isstore);
unsigned char Process(TINYRXINFO* pregex,char c);




unsigned char TINYREGEXProcess(TINYRXINFO* pregex,char c)
{
	volatile char cc = c;
	while (Process(pregex,cc) == RUNITONCEMORE);
	return pregex->IsFound;
}





unsigned char Process(TINYRXINFO* pregex,char cc)
{
	volatile char c=cc;
	char p;
	unsigned char check=REGEXNONE;
	unsigned char issave=REGEXFALSE;

	if ((pregex->IsEnabled)&&(!pregex->IsStopped) )
	{
		p = pregex->Pattern[pregex->PPos];
		switch (p)
		{
			case NEW:
				check = IsLineFeed(c);
				issave=REGEXFALSE;				
				break;
			case ANY:
				check = IsLineFeed(c)  ? REGEXFALSE : REGEXTRUE;
				issave=REGEXTRUE;
				break;
			case NUM:
				check = IsNum(c);
				issave=REGEXTRUE;				
				break;
			case TXT:
				check = (IsNum(c) | IsLineFeed(c)) ? REGEXFALSE : REGEXTRUE;
				issave=REGEXTRUE;				
				break;				
			case SPC:
				check = IsSpc(c);
				issave=REGEXFALSE;				
				break;
			case ALP:
				check = IsSpc(c) ? REGEXFALSE : REGEXTRUE;
				issave=REGEXFALSE;
				break;				
		}

		if (check == REGEXNONE)
		{
			if (c==p)
			{	Move(pregex,REGEXFALSE);
			} else
			{   Stop(pregex,REGEXFALSE);
			}
		} else
		{
			if (check)
			{	CharFound(pregex,c,issave);
			} else
			{	if (pregex->CharsFound!=0)
				{	Move(pregex,issave);
					return 	RUNITONCEMORE;
				} else
				{	Stop(pregex,REGEXFALSE);
				}
			}
		}

	}
	return DONTRUNITAGAIN;
}


unsigned char IsNum(char c)
{
	return ( (c>='0')&&(c<='9')) ? REGEXTRUE : REGEXFALSE;
}

unsigned char IsSpc(char c)
{
	return ( (c==',')||(c==' ')||(c==';')||(c=='.')||(c=='\t')||(c=='\n')||(c=='/')||(c==' '))  ? REGEXTRUE : REGEXFALSE;
}

unsigned char IsLineFeed(char c)
{
	return  (c=='\n')  ? REGEXTRUE : REGEXFALSE;
}





void CharFound(TINYRXINFO* pregex,char c,unsigned char isstore)
{
	if (isstore)
	{	pregex->Match[pregex->MIndex][pregex->MPos] = c;
		pregex->MPos++;
		if (pregex->MPos>=TINYRX_PATTERNMAX) pregex->MPos=0;
	}
	pregex->CharsFound++;
}

void Move(TINYRXINFO* pregex,unsigned char ismovetoanothermatch)
{
	if (pregex->IsStopped) return;
	pregex->PPos++;
    if (pregex->PPos==pregex->PLen) Stop(pregex,REGEXTRUE);
	pregex->CharsFound = 0;
	if (ismovetoanothermatch)
	{
		pregex->MIndex++;
		if (pregex->MIndex>=TINYRX_MATCHMAX) pregex->MIndex=0;
		memset(&(pregex->Match[pregex->MIndex][0]),0,TINYRX_PATTERNMAX);
		pregex->MPos=0;
	}
}

void Stop(TINYRXINFO* pregex,unsigned char result)
{
	 pregex->IsFound = result;
	 pregex->IsStopped = REGEXTRUE;
}


void TINYREGEXClean(TINYRXINFO* pregex)
{
	pregex->IsFound = REGEXFALSE;
	pregex->PPos=0;
	pregex->MPos=0;
	pregex->MIndex=0;
	pregex->IsStopped = REGEXFALSE;
	pregex->IsEnabled = REGEXTRUE;
	memset(pregex->Match,0,TINYRX_MATCHMAX*(TINYRX_PATTERNMAX+1));
}


void TINYREGEXCreate(TINYRXINFO* pregex,char* pattern)
{
	memset(pregex,0,sizeof(TINYRXINFO));
	pregex->IsEnabled =REGEXTRUE;
	pregex->PLen = strlen(pattern);
	if (pregex->PLen>TINYRX_PATTERNMAX)
		pregex->PLen=TINYRX_PATTERNMAX;
	memcpy(pregex->Pattern,pattern,pregex->PLen);

}
