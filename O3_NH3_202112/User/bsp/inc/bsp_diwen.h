#ifndef BSP_DIWEN_H
#define BSP_DIWEN_H	 
	 
void bsp_Diwen_Updatedata(uint16_t dataaddr,uint16_t data_16);	
void bsp_SendKey(uint8_t key);
void bsp_EnableTouch(void);
void bsp_DisableTouch(void);
void bsp_ScreenChange(uint8_t _page);
void bsp_ScreenUPdateSYSSTATUS(uint8_t sta);
extern uint8_t g_page ;
#endif



