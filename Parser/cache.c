#include "cache.h"



void MovePos(PCACHE pcache,unsigned int* pos)
{	*pos+=1;
	if (*pos==pcache->BufSize) *pos=0;
}


void CACHE_Init(PCACHE pcache,char* pbuf,unsigned int bufsize)
{
	pcache->pData = pbuf;
	pcache->BufSize = bufsize;
	CACHE_Clear(pcache);
}

void CACHE_Clear(PCACHE pcache)
{
	pcache->PosIn=0;
	pcache->PosOut=0;
}



void CACHE_Put(PCACHE pcache,char c)
{
	*(pcache->pData+pcache->PosIn)=c;
	MovePos(pcache,&(pcache->PosIn));
	if (pcache->PosIn==pcache->PosOut)
		MovePos(pcache,&(pcache->PosOut));
}


char CACHE_Get(PCACHE pcache)
{
	char c;
	if (CACHE_IsEmpty(pcache)) return 0;
	c = *(pcache->pData + pcache->PosOut);
	MovePos(pcache,&pcache->PosOut);
	return c;
}

unsigned char CACHE_IsEmpty(PCACHE pcache)
{
	return (pcache->PosIn==pcache->PosOut);
}

