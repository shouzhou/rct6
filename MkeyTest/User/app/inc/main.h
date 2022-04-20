/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.h
*	��    �� : V1.4
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/
#ifndef __MAIN_H_
#define __MAIN_H_

#define PRINTF_RX_BUF_SIZE 128
#define PRINTF_TX_BUF_SIZE 128

typedef struct
{
	uint8_t Rxlen;
	char RxBuf[PRINTF_RX_BUF_SIZE];
	uint8_t Txlen;
	char TxBuf[PRINTF_TX_BUF_SIZE];
}PRINT_MODS_T;

extern PRINT_MODS_T g_tPrint;

#endif

 
