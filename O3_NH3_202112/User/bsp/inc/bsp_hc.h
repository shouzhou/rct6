
#ifndef __BSP_HCHC2_H
#define __BSP_HCHC2_H

#include "stdint.h"
#define HC_RX_BUF_SIZE		30
#define HC_TX_BUF_SIZE		30

extern uint16_t g_lenth,g_width,g_height;
extern uint8_t g_hcRevOK,g_FailTime;
extern uint16_t g_mannualtime ,g_loopruntime  ,g_loopstoptime,g_func;
extern uint8_t  g_loopstatus; //0 ������ʱ��� ��1 ������ʱ���
extern uint8_t  g_AutoNh3Status;
typedef struct
{
	uint8_t RxBuf[HC_RX_BUF_SIZE];
	uint8_t RxCount;
	
	uint8_t TxBuf[HC_TX_BUF_SIZE];
	uint8_t TxCount;
}HC_T;
/* ���ⲿ���õĺ������� */
void HC_AskForData(void);
void HC_Analyze(void);

void HC_ReciveNew(uint8_t _data);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);
#endif

