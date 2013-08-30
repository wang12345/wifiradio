#include "uart_parser.h"
#include "tiny_regex.h"
#include <string.h>
#include <stdlib.h>
#include "types.h"


/*
root@OpenWrt:/# mpc next
http://cast.radiogroup.com.ua:8000/retro
[playing] #7/8   0:00/0:00 (100%)
volume: 30%   repeat: on    random: off
root@OpenWrt:/# ~/.mpd/mpc_show.sh
Pos: 6
root@OpenWrt:/# ~/.mpd/mpc_show.sh
Name: RetroFM
Pos: 6
root@OpenWrt:/#

~/.mpd/mpc_update.sh
Connecting to dl.dropboxusercontent.com (54.243.119.191:80)
1.m3u                  0% |                               |     0  --:--:-- ETA~/.mpd/mpc_show.sh
1.m3u                100% |*******************************|   334  --:--:-- ETA
volume: 30%   repeat: on    random: off
volume: 30%   repeat: on    random: off
loading: 1
>1) http://relay3.slayradio.org:8000/
 2) http://87.98.146.216:8000/
 3) http://sfstream1.somafm.com:8884
 4) http://scfire-ntc-aa02.stream.aol.com:80/stream/1010
 5) http://music.myradio.ua/italjanskaja-muzyka128.mp3
 6) http://cast.radiogroup.com.ua:8000/loungefm
 7) http://cast.radiogroup.com.ua:8000/retro
 8) http://cast.radiogroup.com.ua:8000/jamfm
http://relay3.slayradio.org:8000/
[playing] #1/8   0:00/0:00 (100%)
volume: 30%   repeat: on    random: off

Updated
root@OpenWrt:/# ~/.mpd/mpc_show.sh
Pos: 0
root@OpenWrt:/#


status
volume: 10
repeat: 1
random: 0
playlist: 27
playlistlength: 18
xfade: 0
state: play
song: 2
songid: 2
time: 255:0
bitrate: 192
audio: 44100:16:2
OK
currentsong
file: http://sfstream1.somafm.com:8884
Name: Underground Eighties: UK Synthop and a bit of New Wave. [SomaFM]
Title: Kajagoogoo - Hang On Now
Pos: 2
Id: 2
OK

state: stop
*/

#define UP_BOOTSTART "CFE version"
#define UP_BOOTDONE "!!!BOOTDONE"
#define UP_MPCUPDATE "!!!UPDATE"
#define UP_MPCUPDATE_ERROR "!!!U_ERROR"




#define UP_TITLE      "|Title:*|"
#define UP_NAME       "|Name:*|"
#define UP_POS        "|Pos:*|"
#define UP_LISTLEN    "|playlistlength:*|"
#define UP_VOLUME     "|volume:*|"
#define UP_STATE_PLAY "|state: play"
#define UP_STATE_STOP "|state: stop"
#define UP_BITRATE    "|bitrate:*|"


TINYRXINFO _title,_name,_pos,_listlen,_volume,_stateplay,_statestop,_bitrate,_bootstart,_bootdone,_updated,_error;
PUARTSTATUS _pstatus;

#define TRAPMAX 12
TINYRXINFO* _trap[TRAPMAX] = {&_title,&_name,&_pos,&_listlen,&_volume,&_stateplay,&_statestop,&_bitrate,&_bootstart,&_bootdone,&_updated,&_error};

//#define TRAPMAX 1
//TINYRXINFO* _trap[TRAPMAX] = {&_bootdone};

void ProcessTraps();
void SafeCopy(char* from,char* to, int lenmax);


void UP_CreateTraps(PUARTSTATUS status)
{
	TINYREGEXCreate(&_title,UP_TITLE);
	TINYREGEXCreate(&_name,UP_NAME);
	TINYREGEXCreate(&_pos,UP_POS);
	TINYREGEXCreate(&_listlen,UP_LISTLEN);
	TINYREGEXCreate(&_volume,UP_VOLUME);
	TINYREGEXCreate(&_bitrate,UP_BITRATE);
	TINYREGEXCreate(&_stateplay,UP_STATE_PLAY);
	TINYREGEXCreate(&_statestop,UP_STATE_STOP);
	TINYREGEXCreate(&_updated,UP_MPCUPDATE);
	TINYREGEXCreate(&_bootstart,UP_BOOTSTART);
	TINYREGEXCreate(&_bootdone,UP_BOOTDONE);
	TINYREGEXCreate(&_error,UP_MPCUPDATE_ERROR);

	_pstatus = status;
	memset(_pstatus,0,sizeof(UARTSTATUS));
	_pstatus->Pos=NOTSET;
	_pstatus->Volume=NOTSET;
	_pstatus->MaxPos=NOTSET;
	_pstatus->Status = STATUS_UNKNOWN;
	_pstatus->PlayStatus = PLAY_UNKONOWN;
	_pstatus->BitRate = BITRATE_UNKNOWN;

	_pstatus->IsDirty = TRUE;
}


void UP_ClearDirty()
{
	_pstatus->IsDirty = FALSE;
}



void UP_ClearTraps(unsigned char cleartype)
{

	if (cleartype & UP_CLEAR_SCREEN)
	{	TINYREGEXClean(&_title);
		TINYREGEXClean(&_name);
		TINYREGEXClean(&_pos);
		TINYREGEXClean(&_bitrate);
		TINYREGEXClean(&_stateplay);
		TINYREGEXClean(&_statestop);
		TINYREGEXClean(&_bootstart);
		TINYREGEXClean(&_bootdone);
		TINYREGEXClean(&_updated);
		_pstatus->Title[0]=0;
		_pstatus->Name[0]=0;
		_pstatus->Pos=NOTSET;
		_pstatus->ScrollName=0;
		_pstatus->ScrollTitle=0;
		_pstatus->BitRate = BITRATE_UNKNOWN;
		//_pstatus->Status = STATUS_UNKNOWN;
		_pstatus->PlayStatus = PLAY_UNKONOWN;
	}	
	if (cleartype & UP_CLEAR_VOLUME)
	{	TINYREGEXClean(&_volume);
		_pstatus->Volume=NOTSET;
	}	
	if (cleartype & UP_CLEAR_LIST)
	{	TINYREGEXClean(&_listlen);
		_pstatus->MaxPos=NOTSET;
	}	
	_pstatus->IsDirty = TRUE;
}


void UP_ProcessChar(char c)
{
	int i;
	for(i=0;i<TRAPMAX;i++)
		TINYREGEXProcess(_trap[i],c);

	ProcessTraps();
}


void ProcessTraps()
{
	int n,i;


	if (_bootstart.IsFound)
	{
		_pstatus->Status =  STATUS_BOOTING;
		TINYREGEXClean(&_bootstart);
		_pstatus->IsDirty = TRUE;
	}

	if (_bootdone.IsFound)
	{
		_pstatus->Status =  STATUS_BOOTDONE;
		TINYREGEXClean(&_bootdone);
		_pstatus->IsDirty = TRUE;
	}

	if (_title.IsFound)
	{
		SafeCopy(_title.Match[0],_pstatus->Title,UARTSTATUSSTRMAX) ;
		_pstatus->ScrollTitle=0;
		TINYREGEXClean(&_title);
		_pstatus->IsDirty = TRUE;
	}

	if (_name.IsFound)
	{
		SafeCopy(_name.Match[0],_pstatus->Name,UARTSTATUSSTRMAX) ;
		_pstatus->ScrollName=0;
		TINYREGEXClean(&_name);
		_pstatus->IsDirty = TRUE;
		_pstatus->IsNameDirty = TRUE;
	}

	if (_pos.IsFound)
	{
		_pstatus->Pos = atoi(_pos.Match[0]);
		TINYREGEXClean(&_pos);
		_pstatus->IsDirty = TRUE;
	}
	
	if (_listlen.IsFound)
	{
		n = atoi(_listlen.Match[0]);
		TINYREGEXClean(&_listlen);
		if ((_pstatus->MaxPos==NOTSET) ||(_pstatus->MaxPos<n)) _pstatus->MaxPos=n;
		_pstatus->IsDirty = TRUE;
	}

	if (_volume.IsFound)
	{
		_pstatus->Volume = atoi(_volume.Match[0]);
		TINYREGEXClean(&_volume);
		_pstatus->IsDirty = TRUE;
	}

	
	if (_bitrate.IsFound)
	{
		_pstatus->BitRate = atoi(_bitrate.Match[0]);
		TINYREGEXClean(&_bitrate);
		_pstatus->IsDirty = TRUE;
	}

	if (_stateplay.IsFound)
	{
		_pstatus->PlayStatus = PLAY_YES;
		TINYREGEXClean(&_stateplay);
		_pstatus->IsDirty = TRUE;
	}

	if (_statestop.IsFound)
	{
		_pstatus->PlayStatus = PLAY_NO;
		TINYREGEXClean(&_statestop);
		_pstatus->IsDirty = TRUE;
	}

	if (_updated.IsFound)
	{
		_pstatus->Status = STATUS_READY;
		TINYREGEXClean(&_updated);
		_pstatus->IsDirty = TRUE;
	}

	if (_error.IsFound)
	{
		_pstatus->Status = STATUS_ERROR;
		TINYREGEXClean(&_error);
		_pstatus->IsDirty = TRUE;
	}

	for(i=0;i<TRAPMAX;i++)
		if (_trap[i]->IsStopped) TINYREGEXClean(_trap[i]);

}



void SafeCopy(char* from,char* to, int lenmax)
{
	int i;
	for(i=0;i<lenmax-1;i++)
	{	*(to+i) = *(from+i);
		if (*(from+i) == 0) return;
	}
	*(to+lenmax)=0;

}

