#ifndef BSP_DIWEN_H
#define BSP_DIWEN_H	 
#include "stdint.h"	 
void bsp_Diwen_Updatedata(uint16_t dataaddr,uint16_t data_16);	
void bsp_SendKey(uint8_t key);
void bsp_EnableTouch(void);
void bsp_DisableTouch(void);
void bsp_ScreenChange(uint8_t _page);

void bsp_ScreenUpdateText(void);
void bsp_ScreenUPdateSYSSTATUS(uint8_t _sta); //系统信息显示
void bsp_ScreenUpdateNet(uint8_t _sta,uint8_t withoutnet); //onenet状态显示
void bsp_ScreenUPdateType(uint8_t plant); //植物信息显示
void bsp_ScreenUPdateChemical(uint8_t chemical); //植物信息显示
void bsp_PlayMusic(void);
#endif



