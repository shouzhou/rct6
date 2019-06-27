
#ifndef __BSP_HCHC2_H
#define __BSP_HCHC2_H

#include "stdint.h"
#define HC_RX_BUF_SIZE		30
#define HC_TX_BUF_SIZE		30

extern uint16_t g_lenth,g_width,g_height;
extern uint8_t g_hcRevOK;
typedef struct
{
	uint8_t RxBuf[HC_RX_BUF_SIZE];
	uint8_t RxCount;
	
	uint8_t TxBuf[HC_TX_BUF_SIZE];
	uint8_t TxCount;
}HC_T;
/* 供外部调用的函数声明 */
void HC_AskForData(void);
void HC_Analyze(void);

void HC_ReciveNew(uint8_t _data);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);
#endif


