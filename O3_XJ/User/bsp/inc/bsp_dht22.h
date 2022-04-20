/*
*********************************************************************************************************
*
*	ģ������ : DHT11 ����ģ��(1-wire ������ʪ�ȴ�������
*	�ļ����� : bsp_dht11.h
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_DHT11_H
#define _BSP_DHT11_H

typedef struct
{
	uint8_t Buf[5];
	uint16_t Temp;		/* Temperature �¶� ���϶� */
	uint16_t Hum;		/* Humidity ʪ�� �ٷֱ� */
}DHT11_T;

void bsp_InitDHT11(void);
uint8_t DHT11_ReadData(DHT11_T *_pDHT);
extern  DHT11_T  g_DHT11;
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
