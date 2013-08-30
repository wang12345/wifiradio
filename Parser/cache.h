#ifndef UARTCACHE_H_
#define UARTCACHE_H_




typedef struct tagCACHE
{
	char* pData;
	unsigned int BufSize;
	unsigned int PosIn;
	unsigned int PosOut;

} CACHE, *PCACHE;


void CACHE_Init(PCACHE pcache,char* pbuf,unsigned int bufsize);
void CACHE_Clear(PCACHE pcache);
void CACHE_Put(PCACHE pcache,char c);
char CACHE_Get(PCACHE pcache);
unsigned char CACHE_IsEmpty(PCACHE pcache);







#endif

