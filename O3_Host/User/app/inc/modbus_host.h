/*
*********************************************************************************************************
*
*	ģ������ : MODEBUS ͨ��ģ�� (��������
*	�ļ����� : modbus_host.h
*	��    �� : V1.4
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __MOSBUS_HOST_H
#define __MOSBUS_HOST_H

#define SlaveAddr		0x01			/* �����Ϊʱ���������ӻ� */
#define HBAUD485		UART2_BAUD

/* 01H ��ǿ�Ƶ���Ȧ */
/* 05H дǿ�Ƶ���Ȧ */
#define REG_D01		0x0101
#define REG_D02		0x0102
#define REG_D03		0x0103
#define REG_D04		0x0104
#define REG_DXX 	REG_D04

/* 02H ��ȡ����״̬ */
#define REG_T01		0x0201
#define REG_T02		0x0202
#define REG_T03		0x0203
#define REG_TXX		REG_T03

/* 03H �����ּĴ��� */
/* 06H д���ּĴ��� */
/* 10H д�������Ĵ��� */
#define REG_P01		0x0000		
#define REG_P02		0x0001	
#define REG_P03		0x0002		
#define REG_P04		0x0003
#define REG_P05		0x0004		
#define REG_P06		0x0005
#define REG_P07		0x0006		
#define REG_P08		0x0007

/* 04H ��ȡ����Ĵ���(ģ���ź�) */
#define REG_A01		0x0401
#define REG_AXX		REG_A01

/* RTU Ӧ����� */
#define RSP_OK				0		/* �ɹ� */
#define RSP_ERR_CMD			0x01	/* ��֧�ֵĹ����� */
#define RSP_ERR_REG_ADDR	0x02	/* �Ĵ�����ַ���� */
#define RSP_ERR_VALUE		0x03	/* ����ֵ����� */
#define RSP_ERR_WRITE		0x04	/* д��ʧ�� */

#define H_RX_BUF_SIZE		64
#define H_TX_BUF_SIZE      	128

typedef struct
{
	uint8_t RxBuf[H_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[H_TX_BUF_SIZE];
	uint8_t TxCount;
	
	uint16_t Reg01H;		/* �����������͵ļĴ����׵�ַ */
	uint16_t Reg02H;
	uint16_t Reg03H;		
	uint16_t Reg04H;

	uint8_t RegNum;			/* �Ĵ������� */

	uint8_t fAck01H;		/* Ӧ�������־ 0 ��ʾִ��ʧ�� 1��ʾִ�гɹ� */
	uint8_t fAck02H;
	uint8_t fAck03H;
	uint8_t fAck04H;
	uint8_t fAck05H;		
	uint8_t fAck06H;		
	uint8_t fAck10H;
	
}MODH_T;

typedef struct
{
	/* 03H 06H ��д���ּĴ��� */
	uint16_t P01;
	uint16_t P02;
	
	/* 02H ��д��ɢ����Ĵ��� */
	uint16_t T01;
	uint16_t T02;
	uint16_t T03;
	
	/* 04H ��ȡģ�����Ĵ��� */
	uint16_t A01;
	
	/* 01H 05H ��д����ǿ����Ȧ */
	uint16_t D01;
	uint16_t D02;
	uint16_t D03;
	uint16_t D04;
	
}VAR_T;

void MODH_Poll(void);
uint8_t MODH_ReadParam_01H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_02H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_04H(uint16_t _reg, uint16_t _num);
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value);
uint8_t MODH_WriteParam_06H(uint16_t _reg, uint16_t _value);
uint8_t MODH_WriteParam_10H(uint16_t _reg, uint8_t _num, uint8_t *_buf);

extern MODH_T g_tModH;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
