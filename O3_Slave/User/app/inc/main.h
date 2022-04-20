/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.h
*	版    本 : V1.4
*	说    明 : 头文件
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
