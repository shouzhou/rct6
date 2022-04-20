
#ifndef __BSP_HCHC2_H
#define __BSP_HCHC2_H


#define HC_RX_BUF_SIZE		128
#define HC_TX_BUF_SIZE		30

extern uint8_t g_hcRevOK;
extern uint8_t g_hcDataOk,g_hcStable;
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
void bsp_HCDataProcess(uint16_t hum,uint16_t temp);
#endif


