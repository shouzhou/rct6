/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.h
*	��    �� : V1.4
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __MAIN_H_
#define __MAIN_H_

typedef struct
{
	uint8_t Rxlen;
	char RxBuf[S_RX_BUF_SIZE];
	uint8_t Txlen;
	char TxBuf[S_TX_BUF_SIZE];
}PRINT_MODS_T;

extern PRINT_MODS_T g_tPrint;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
