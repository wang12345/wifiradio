#ifndef UARTPARSER_H_
#define UARTPARSER_H_

#define PLAY_UNKONOWN   0
#define PLAY_YES        1
#define PLAY_NO         2

#define STATUS_UNKNOWN  0
#define STATUS_BOOTING  3
#define STATUS_BOOTDONE 4
#define STATUS_LOADING  5
#define STATUS_READY    6
#define STATUS_WORKING  7
#define STATUS_SELECTCH 8
#define STATUS_ERROR    9



#define BITRATE_UNKNOWN 0


#define UP_CLEAR_SCREEN 0x01
#define UP_CLEAR_VOLUME 0x02
#define UP_CLEAR_LIST   0x04
#define UP_CLEAR_ALL    0x0F


#define UARTSTATUSSTRMAX 256


typedef struct tagUARTSTATUS
{
	char Title[UARTSTATUSSTRMAX+1];
	char Name[UARTSTATUSSTRMAX+1];
	int Status;
	int BitRate;
	int Pos;
	int MaxPos;
	int Volume;
	int IsDirty;
	int IsNameDirty;
	int PlayStatus;
	unsigned int ScrollTitle;
	unsigned int ScrollName;


} UARTSTATUS, *PUARTSTATUS;


void UP_CreateTraps(PUARTSTATUS status);
void UP_ClearTraps(unsigned char cleartype);
void UP_ProcessChar(char c);
void UP_ClearDirty();


#endif

