/*
*********************************************************************************************************
*
*	ģ������ : MODEBUS ͨ��ģ�� (��վ��
*	�ļ����� : modbus_slave.h
*	��    �� : V1.4
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MODBUY_SLAVE_H
#define __MODBUY_SLAVE_H


#define SBAUD485	    UART3_BAUD
#define SLAVEADDRESS    3
/* 01H ��ǿ�Ƶ���Ȧ */
/* 05H дǿ�Ƶ���Ȧ */
#define REG_D01		0x0001
#define REG_D02		0x0002
#define REG_D03		0x0003
#define REG_D04		0x0004
#define REG_DXX 	REG_D04

/* 02H ��ȡ����״̬ */
#define REG_T01		0x0201
#define REG_T02		0x0202
#define REG_T03		0x0203
#define REG_TXX		REG_T03

/* 03H �����ּĴ��� */
/* 06H д���ּĴ��� */
/* 10H д�������Ĵ��� */
#define SLAVE_REG_P01		0x0000
#define SLAVE_REG_P02		0x0001
#define SLAVE_REG_P03		0x0002
#define SLAVE_REG_P04		0x0003
#define SLAVE_REG_P05		0x0004
#define SLAVE_REG_P06		0x0005
#define SLAVE_REG_P07		0x0006
#define SLAVE_REG_P08		0x0007
#define SLAVE_REG_P09		0x0008
#define SLAVE_REG_P10		0x0009
#define SLAVE_REG_P11		0x000A
#define SLAVE_REG_P12		0x000B
#define SLAVE_REG_P13		0x000C
#define SLAVE_REG_P14		0x000D
#define SLAVE_REG_P15		0x000E
#define SLAVE_REG_P16		0x000F
#define SLAVE_REG_P17		0x0010
#define SLAVE_REG_P18		0x0011
#define SLAVE_REG_P19		0x0012
#define SLAVE_REG_P20		0x0013
#define SLAVE_REG_P21		0x0014
#define SLAVE_REG_P22		0x0015
#define SLAVE_REG_P23		0x0016
#define SLAVE_REG_P24		0x0017
#define SLAVE_REG_P25		0x0018
#define SLAVE_REG_P26		0x0019
#define SLAVE_REG_P27		0x001A
#define SLAVE_REG_P28		0x001B
#define SLAVE_REG_P29		0x001C
#define SLAVE_REG_P30		0x001D


/* 04H ��ȡ����Ĵ���(ģ���ź�) */
#define REG_A01		0x0401
#define REG_AXX		REG_A01


/* RTU Ӧ����� */
#define RSP_OK				0		/* �ɹ� */
#define RSP_ERR_CMD			0x01	/* ��֧�ֵĹ����� */
#define RSP_ERR_REG_ADDR	0x02	/* �Ĵ�����ַ���� */
#define RSP_ERR_VALUE		0x03	/* ����ֵ����� */
#define RSP_ERR_WRITE		0x04	/* д��ʧ�� */

#define S_RX_BUF_SIZE		64
#define S_TX_BUF_SIZE		128

typedef struct
{
	uint8_t RxBuf[S_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[S_TX_BUF_SIZE];
	uint8_t TxCount;
}MODS_T;

typedef struct
{
	/* 03H 06H 10H��д���ּĴ��� */
    //  P01 --P20   �������Ӵӻ������ȡ
    ///////////////////////////////////////////
    //  P01 ---�ӻ���
    //  P02 ---�¶�ֵ--�Ŵ�100��
    //  P03 ---ʪ��ֵ--�Ŵ�100��
    //  P04 ---����Ũ��
    //  P05 ---����Ũ��
    //  P06 ---
    //////////////////////////////////////
    //  P21 --P30   ����д��ӻ�����
////////////////////////////////////////////////
    //  P21 ---���� ϵͳ�Ƿ�����ͣ�������������и�д  0x5AA5  ����    0X A55A  ֹͣ
    //  P22 ---����ģʽ  2�� 3��4 
    //  P23 ---��
    //  P24 ---��
    //  P25 ---��
    //  P26 ---����
	uint16_t P01;
	uint16_t P02;
    uint16_t P03;
	uint16_t P04;
    uint16_t P05;
	uint16_t P06;
    uint16_t P07;
    uint16_t P08;
    uint16_t P09;
	uint16_t P10;
    uint16_t P11;
	uint16_t P12;
    uint16_t P13;
	uint16_t P14;
    uint16_t P15;
    uint16_t P16;
    uint16_t P17;
	uint16_t P18;
    uint16_t P19;
	uint16_t P20;
    uint16_t P21;
	uint16_t P22;
    uint16_t P23;
    uint16_t P24;
    uint16_t P25;
	uint16_t P26;
    uint16_t P27;
	uint16_t P28;
    uint16_t P29;
	uint16_t P30;
  
	

	/* 04H ��ȡģ�����Ĵ��� */
	uint16_t A01;

	/* 01H 05H ��д����ǿ����Ȧ */
	uint16_t D01;
	uint16_t D02;
	uint16_t D03;
	uint16_t D04;
    uint16_t D05;

}VAR_T;

void MODS_Poll(void);

extern MODS_T g_tModS;
extern VAR_T g_tVar;
extern uint8_t  SADDR485;

void setDefaultvalue(void);
extern uint8_t SADDR485;
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
