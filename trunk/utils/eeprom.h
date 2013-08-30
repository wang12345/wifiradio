







void EEPROM_Init();

int  EEPROM_GetVolume();
void EEPROM_PutVolume(int volume);
int  EEPROM_GetPos();
void EEPROM_PutPos(int pos);
void EEPROM_PutTitle(char* ptitle,int index,int maxlen);
void EEPROM_GetTitle(char* ptitle,int index,int maxlen);
unsigned char EEPROM_IsActive();
unsigned char EEPROM_Test();
