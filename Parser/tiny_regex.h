#ifndef TINYRX_H_
#define TINYRX_H_

#define TINYRX_PATTERNMAX 256
#define TINYRX_MATCHSTRMAX 256
#define TINYRX_MATCHMAX 3

typedef struct tagTINYRXINFO
{
	unsigned short int PPos;
	unsigned char IsEnabled;
	unsigned char IsFound;
	unsigned short int MPos;
	unsigned short int MIndex;
	unsigned char IsStopped;
	unsigned short int PLen;
	unsigned short int CharsFound;
	char Pattern[TINYRX_PATTERNMAX+1];
	char Match[TINYRX_MATCHMAX][TINYRX_MATCHSTRMAX +1];

} TINYRXINFO, *PTINYRXINFO;


unsigned char TINYREGEXProcess(TINYRXINFO* pregex,char c);
void TINYREGEXClean(TINYRXINFO* pregex);
void TINYREGEXCreate(TINYRXINFO* pregex,char* pattern);


#endif

