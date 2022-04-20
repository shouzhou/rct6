/*
*********************************************************************************************************
*
*	ģ������ : MODSͨ��ģ��. ��վģʽ
*	�ļ����� : modbus_slave.c
*	��    �� : V1.4
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2016,    
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "modbus_slave.h"
#include "main.h"

static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen);
static void MODS_SendAckOk(void);
static void MODS_SendAckErr(uint8_t _ucErrCode);

static void MODS_AnalyzeApp(void);

static void MODS_RxTimeOut(void);

static void MODS_01H(void);
static void MODS_02H(void);
static void MODS_03H(void);
static void MODS_04H(void);
static void MODS_05H(void);
static void MODS_06H(void);
static void MODS_10H(void);

static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value);
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value);

static uint8_t g_mods_timeout = 0;
void MODS_ReciveNew(uint8_t _byte);
uint8_t SADDR485=0x01;
MODS_T g_tModS;
VAR_T g_tVar;
/*
*********************************************************************************************************
*	�� �� ��: MODS_Poll
*	����˵��: �������ݰ�. �����������������á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODS_Poll(void)
{
	uint16_t addr;
	uint16_t crc1;
	/* ����3.5���ַ�ʱ���ִ��MODH_RxTimeOut()������ȫ�ֱ��� g_rtu_timeout = 1; ֪ͨ������ʼ���� */
	if (g_mods_timeout == 0)	
	{
		return;								/* û�г�ʱ���������ա���Ҫ���� g_tModS.RxCount */
	}
	
	g_mods_timeout = 0;	 					/* ���־ */

	if (g_tModS.RxCount < 4)				/* ���յ�������С��4���ֽھ���Ϊ���� */
	{
		goto err_ret;
	}

	/* ����CRCУ��� */
	crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* վ��ַ (1�ֽڣ� */
	addr = g_tModS.RxBuf[0];				/* ��1�ֽ� վ�� */
	if (addr != SADDR485)		 			/* �ж��������͵������ַ�Ƿ���� */
	{
		goto err_ret;
	}

	/* ����Ӧ�ò�Э�� */
	MODS_AnalyzeApp();						
	bsp_LedToggle(2); //MODBUS ͨѶһ�� ��˸һ��D5
err_ret:
#if 0										/* �˲���Ϊ�˴��ڴ�ӡ���,ʵ�������пɲ�Ҫ */
	g_tPrint.Rxlen = g_tModS.RxCount;
	memcpy(g_tPrint.RxBuf, g_tModS.RxBuf, g_tModS.RxCount);
#endif
	
	g_tModS.RxCount = 0;					/* ��������������������´�֡ͬ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_ReciveNew
*	����˵��: ���ڽ����жϷ���������ñ����������յ�һ���ֽ�ʱ��ִ��һ�α�������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t _byte)
{
	/*
		3.5���ַ���ʱ������ֻ������RTUģʽ���棬��ΪRTUģʽû�п�ʼ���ͽ�������
		�������ݰ�֮��ֻ�ܿ�ʱ���������֣�Modbus�����ڲ�ͬ�Ĳ������£����ʱ���ǲ�һ���ģ�
		���Ծ���3.5���ַ���ʱ�䣬�����ʸߣ����ʱ������С�������ʵͣ����ʱ������Ӧ�ʹ�

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
	uint32_t timeout;

	g_mods_timeout = 0;
	
//	timeout = 35000000 / SBAUD485;			/* ���㳬ʱʱ�䣬��λus 35000000*/
    timeout = 45000000 / SBAUD485;			/* ���㳬ʱʱ�䣬��λus 35000000*/
	
	/* Ӳ����ʱ�жϣ���ʱ����us Ӳ����ʱ��1����ADC, ��ʱ��2����Modbus */
	bsp_StartHardTimer(1, timeout, (void *)MODS_RxTimeOut);

	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_RxTimeOut
*	����˵��: ����3.5���ַ�ʱ���ִ�б������� ����ȫ�ֱ��� g_mods_timeout = 1; ֪ͨ������ʼ���롣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_RxTimeOut(void)
{
	g_mods_timeout = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendWithCRC
*	����˵��: ����һ������, �Զ�׷��2�ֽ�CRC
*	��    ��: _pBuf ���ݣ�
*			  _ucLen ���ݳ��ȣ�����CRC��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen)
{
	uint16_t crc;
	uint8_t buf[S_TX_BUF_SIZE];

	memcpy(buf, _pBuf, _ucLen);
	crc = CRC16_Modbus(_pBuf, _ucLen);
	buf[_ucLen++] = crc >> 8;
	buf[_ucLen++] = crc;

	RS485_SendBuf(buf, _ucLen);
	
#if 0									/* �˲���Ϊ�˴��ڴ�ӡ���,ʵ�������пɲ�Ҫ */
	g_tPrint.Txlen = _ucLen;
	memcpy(g_tPrint.TxBuf, buf, _ucLen);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckErr
*	����˵��: ���ʹ���Ӧ��
*	��    ��: _ucErrCode : �������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = g_tModS.RxBuf[0];					/* 485��ַ */
	txbuf[1] = g_tModS.RxBuf[1] | 0x80;				/* �쳣�Ĺ����� */
	txbuf[2] = _ucErrCode;							/* �������(01,02,03,04) */

	MODS_SendWithCRC(txbuf, 3);
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckOk
*	����˵��: ������ȷ��Ӧ��.
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_SendAckOk(void)
{
	uint8_t txbuf[6];
	uint8_t i;

	for (i = 0; i < 6; i++)
	{
		txbuf[i] = g_tModS.RxBuf[i];
	}
	MODS_SendWithCRC(txbuf, 6);
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_AnalyzeApp
*	����˵��: ����Ӧ�ò�Э��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_AnalyzeApp(void)
{
	switch (g_tModS.RxBuf[1])				/* ��2���ֽ� ������ */
	{
		case 0x01:							/* ��ȡ��Ȧ״̬����������led���棩*/
			MODS_01H();
			bsp_PutMsg(MSG_MODS_01H, 0);	/* ������Ϣ,�������� */
			break;

		case 0x02:							/* ��ȡ����״̬������״̬��*/
			MODS_02H();
			bsp_PutMsg(MSG_MODS_02H, 0);
			break;
		
		case 0x03:							/* ��ȡ���ּĴ����������̴���g_tVar�У�*/
			MODS_03H();
			bsp_PutMsg(MSG_MODS_03H, 0);
			break;
		
		case 0x04:							/* ��ȡ����Ĵ�����ADC��ֵ��*/
			MODS_04H();
			bsp_PutMsg(MSG_MODS_04H, 0);
			break;
		
		case 0x05:							/* ǿ�Ƶ���Ȧ������led��*/
			MODS_05H();
			bsp_PutMsg(MSG_MODS_05H, 0);
			break;
		
		case 0x06:							/* д��������Ĵ����������̸�дg_tVar�еĲ�����*/
			MODS_06H();	
			bsp_PutMsg(MSG_MODS_06H, 0);
			break;
			
		case 0x10:							/* д�������Ĵ����������̴���g_tVar�еĲ�����*/
			MODS_10H();
			bsp_PutMsg(MSG_MODS_10H, 0);
			break;
		
		default:
			g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* ��������������� */
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_01H
*	����˵��: ��ȡ��Ȧ״̬����ӦԶ�̿���D01/D02/D03��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/* ˵��:������LED����̵���,���ڹ۲����� */
static void MODS_01H(void)
{
	/*
	 ������
		��������:
			11 �ӻ���ַ
			01 ������
			00 �Ĵ�����ʼ��ַ���ֽ�
			13 �Ĵ�����ʼ��ַ���ֽ�
			00 �Ĵ����������ֽ�
			25 �Ĵ����������ֽ�
			0E CRCУ����ֽ�
			84 CRCУ����ֽ�

		�ӻ�Ӧ��: 	1����ON��0����OFF�������ص���Ȧ����Ϊ8�ı�����������������ֽ�δβʹ��0����. BIT0��Ӧ��1��
			11 �ӻ���ַ
			01 ������
			05 �����ֽ���
			CD ����1(��Ȧ0013H-��Ȧ001AH)
			6B ����2(��Ȧ001BH-��Ȧ0022H)
			B2 ����3(��Ȧ0023H-��Ȧ002AH)
			0E ����4(��Ȧ0032H-��Ȧ002BH)
			1B ����5(��Ȧ0037H-��Ȧ0033H)
			45 CRCУ����ֽ�
			E6 CRCУ����ֽ�

		����:
			01 01 10 01 00 03   29 0B	--- ��ѯD01��ʼ��3���̵���״̬
			01 01 10 03 00 01   09 0A   --- ��ѯD03�̵�����״̬
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];
	
	g_tModS.RspCode = RSP_OK;

	/* û���ⲿ�̵�����ֱ��Ӧ����� */
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;				/* ����ֵ����� */
		return;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* �Ĵ����� */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* �Ĵ������� */

	m = (num + 7) / 8;
	
	if ((reg >= REG_D01) && (num > 0) && (reg + num <= REG_DXX + 1))
	{
		for (i = 0; i < m; i++)
		{
			status[i] = 0;
		}
		for (i = 0; i < num; i++)
		{
			if (bsp_IsLedOn(i + 1 + reg - REG_D01))		/* ��LED��״̬��д��״̬�Ĵ�����ÿһλ */
			{  
				status[i / 8] |= (1 << (i % 8));
			}
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* �Ĵ�����ַ���� */
	}

	if (g_tModS.RspCode == RSP_OK)						/* ��ȷӦ�� */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = m;			/* �����ֽ��� */

		for (i = 0; i < m; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];	/* �̵���״̬ */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);				/* ��������������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_02H
*	����˵��: ��ȡ����״̬����ӦK01��K03��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_02H(void)
{
	/*
		��������:
			11 �ӻ���ַ
			02 ������
			00 �Ĵ�����ַ���ֽ�
			C4 �Ĵ�����ַ���ֽ�
			00 �Ĵ����������ֽ�
			16 �Ĵ����������ֽ�
			BA CRCУ����ֽ�
			A9 CRCУ����ֽ�

		�ӻ�Ӧ��:  ��Ӧ����ɢ����Ĵ���״̬���ֱ��Ӧ�������е�ÿλֵ��1 ����ON��0 ����OFF��
		           ��һ�������ֽڵ�LSB(����ֽ�)Ϊ��ѯ��Ѱַ��ַ����������ڰ�˳���ڸ��ֽ����ɵ��ֽ�
		           ����ֽ����У�ֱ�������8λ����һ���ֽ��е�8������λҲ�Ǵӵ��ֽڵ����ֽ����С�
		           �����ص�����λ������8�ı������������������ֽ��е�ʣ��λ�����ֽڵ����λʹ��0��䡣
			11 �ӻ���ַ
			02 ������
			03 �����ֽ���
			AC ����1(00C4H-00CBH)
			DB ����2(00CCH-00D3H)
			35 ����3(00D4H-00D9H)
			20 CRCУ����ֽ�
			18 CRCУ����ֽ�

		����:
		01 02 20 01 00 08  23CC  ---- ��ȡT01-08��״̬
		01 02 20 04 00 02  B3CA  ---- ��ȡT04-05��״̬
		01 02 20 01 00 12  A207   ---- �� T01-18
	*/

	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;				/* ����ֵ����� */
		return;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* �Ĵ����� */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* �Ĵ������� */

	m = (num + 7) / 8;
	if ((reg >= REG_T01) && (num > 0) && (reg + num <= REG_TXX + 1))
	{
		for (i = 0; i < m; i++)
		{
			status[i] = 0;
		}
		for (i = 0; i < num; i++)
		{
			if (bsp_GetKeyState((KEY_ID_E)(KID_K1 + reg - REG_T01 + i)))
			{
				status[i / 8] |= (1 << (i % 8));
			}
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* �Ĵ�����ַ���� */
	}

	if (g_tModS.RspCode == RSP_OK)						/* ��ȷӦ�� */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = m;			/* �����ֽ��� */

		for (i = 0; i < m; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];	/* T01-02״̬ */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);				/* ��������������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_ReadRegValue
*	����˵��: ��ȡ���ּĴ�����ֵ
*	��    ��: reg_addr �Ĵ�����ַ
*			  reg_value ��żĴ������
*	�� �� ֵ: 1��ʾOK 0��ʾ����
*********************************************************************************************************
*/
static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
	uint16_t value;
	
	switch (reg_addr)									/* �жϼĴ�����ַ */
	{
		case SLAVE_REG_P01:
			value =	g_tVar.P01;	
			break;

		case SLAVE_REG_P02:
			value =	g_tVar.P02;							/* ���Ĵ���ֵ���� */
			break;
	
		default:
			return 0;									/* �����쳣������ 0 */
	}

	reg_value[0] = value >> 8;
	reg_value[1] = value;

	return 1;											/* ��ȡ�ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_WriteRegValue
*	����˵��: ��ȡ���ּĴ�����ֵ
*	��    ��: reg_addr �Ĵ�����ַ
*			  reg_value �Ĵ���ֵ
*	�� �� ֵ: 1��ʾOK 0��ʾ����
*********************************************************************************************************
*/
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value)
{
	switch (reg_addr)							/* �жϼĴ�����ַ */
	{	
		case SLAVE_REG_P01:
			g_tVar.P01 = reg_value;				/* ��ֵд�뱣��Ĵ��� */
			break;
		
		case SLAVE_REG_P02:
			g_tVar.P02 = reg_value;				/* ��ֵд�뱣��Ĵ��� */
			break;
		
		default:
			return 0;		/* �����쳣������ 0 */
	}

	return 1;		/* ��ȡ�ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_03H
*	����˵��: ��ȡ���ּĴ��� ��һ���������ּĴ�����ȡ�õ�ǰ�Ķ�����ֵ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_03H(void)
{
	/*
		�ӻ���ַΪ11H�����ּĴ�������ʼ��ַΪ006BH��������ַΪ006DH���ôβ�ѯ�ܹ�����3�����ּĴ�����

		��������:
			11 �ӻ���ַ
			03 ������
			00 �Ĵ�����ַ���ֽ�
			6B �Ĵ�����ַ���ֽ�
			00 �Ĵ����������ֽ�
			03 �Ĵ����������ֽ�
			76 CRC���ֽ�
			87 CRC���ֽ�

		�ӻ�Ӧ��: 	���ּĴ����ĳ���Ϊ2���ֽڡ����ڵ������ּĴ������ԣ��Ĵ������ֽ������ȱ����䣬
					���ֽ����ݺ󱻴��䡣���ּĴ���֮�䣬�͵�ַ�Ĵ����ȱ����䣬�ߵ�ַ�Ĵ����󱻴��䡣
			11 �ӻ���ַ
			03 ������
			06 �ֽ���
			00 ����1���ֽ�(006BH)
			6B ����1���ֽ�(006BH)
			00 ����2���ֽ�(006CH)
			13 ����2 ���ֽ�(006CH)
			00 ����3���ֽ�(006DH)
			00 ����3���ֽ�(006DH)
			38 CRC���ֽ�
			B9 CRC���ֽ�

		����:
			01 03 30 06 00 01  6B0B      ---- �� 3006H, ��������
			01 03 4000 0010 51C6         ---- �� 4000H ������1����ӿ��¼ 32�ֽ�
			01 03 4001 0010 0006         ---- �� 4001H ������1����ӿ��¼ 32�ֽ�

			01 03 F000 0008 770C         ---- �� F000H ������1���澯��¼ 16�ֽ�
			01 03 F001 0008 26CC         ---- �� F001H ������2���澯��¼ 16�ֽ�

			01 03 7000 0020 5ED2         ---- �� 7000H ������1�����μ�¼��1�� 64�ֽ�
			01 03 7001 0020 0F12         ---- �� 7001H ������1�����μ�¼��2�� 64�ֽ�

			01 03 7040 0020 5F06         ---- �� 7040H ������2�����μ�¼��1�� 64�ֽ�
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t reg_value[64];

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8)								/* 03H���������8���ֽ� */
	{
		g_tModS.RspCode = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 				/* �Ĵ����� */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);					/* �Ĵ������� */
	if (num > sizeof(reg_value) / 2)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}

	for (i = 0; i < num; i++)
	{
		if (MODS_ReadRegValue(reg, &reg_value[2 * i]) == 0)	/* �����Ĵ���ֵ����reg_value */
		{
			g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* �Ĵ�����ַ���� */
			break;
		}
		reg++;
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK)							/* ��ȷӦ�� */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			/* �����ֽ��� */

		for (i = 0; i < num; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i];
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i+1];
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);	/* ������ȷӦ�� */
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);					/* ���ʹ���Ӧ�� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_04H
*	����˵��: ��ȡ����Ĵ�������ӦA01/A02�� SMA
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_04H(void)
{
	/*
		��������:
			11 �ӻ���ַ
			04 ������
			00 �Ĵ�����ʼ��ַ���ֽ�
			08 �Ĵ�����ʼ��ַ���ֽ�
			00 �Ĵ����������ֽ�
			02 �Ĵ����������ֽ�
			F2 CRC���ֽ�
			99 CRC���ֽ�

		�ӻ�Ӧ��:  ����Ĵ�������Ϊ2���ֽڡ����ڵ�������Ĵ������ԣ��Ĵ������ֽ������ȱ����䣬
				���ֽ����ݺ󱻴��䡣����Ĵ���֮�䣬�͵�ַ�Ĵ����ȱ����䣬�ߵ�ַ�Ĵ����󱻴��䡣
			11 �ӻ���ַ
			04 ������
			04 �ֽ���
			00 ����1���ֽ�(0008H)
			0A ����1���ֽ�(0008H)
			00 ����2���ֽ�(0009H)
			0B ����2���ֽ�(0009H)
			8B CRC���ֽ�
			80 CRC���ֽ�

		����:

			01 04 2201 0006 2BB0  --- �� 2201H A01ͨ��ģ���� ��ʼ��6������
			01 04 2201 0001 6A72  --- �� 2201H

	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t status[10];

	memset(status, 0, 10);

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;	/* ����ֵ����� */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* �Ĵ����� */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);	/* �Ĵ������� */
	
	if ((reg >= REG_A01) && (num > 0) && (reg + num <= REG_AXX + 1))
	{	
		for (i = 0; i < num; i++)
		{
			switch (reg)
			{
				/* ���Բ��� */
				case REG_A01:
					status[i] = g_tVar.A01;
					break;
					
				default:
					status[i] = 0;
					break;
			}
			reg++;
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* �Ĵ�����ַ���� */
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK)		/* ��ȷӦ�� */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			/* �����ֽ��� */

		for (i = 0; i < num; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] >> 8;
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] & 0xFF;
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);	/* ��������������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_05H
*	����˵��: ǿ�Ƶ���Ȧ����ӦD01/D02/D03��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_05H(void)
{
	/*
		��������: д������Ȧ�Ĵ�����FF00Hֵ������Ȧ����ON״̬��0000Hֵ������Ȧ����OFF״̬
		��05Hָ�����õ�����Ȧ��״̬��15Hָ��������ö����Ȧ��״̬��
			11 �ӻ���ַ
			05 ������
			00 �Ĵ�����ַ���ֽ�
			AC �Ĵ�����ַ���ֽ�
			FF ����1���ֽ�
			00 ����2���ֽ�
			4E CRCУ����ֽ�
			8B CRCУ����ֽ�

		�ӻ�Ӧ��:
			11 �ӻ���ַ
			05 ������
			00 �Ĵ�����ַ���ֽ�
			AC �Ĵ�����ַ���ֽ�
			FF �Ĵ���1���ֽ�
			00 �Ĵ���1���ֽ�
			4E CRCУ����ֽ�
			8B CRCУ����ֽ�

		����:
		01 05 10 01 FF 00   D93A   -- D01��
		01 05 10 01 00 00   98CA   -- D01�ر�

		01 05 10 02 FF 00   293A   -- D02��
		01 05 10 02 00 00   68CA   -- D02�ر�

		01 05 10 03 FF 00   78FA   -- D03��
		01 05 10 03 00 00   390A   -- D03�ر�
	*/
	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;
	
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* ����ֵ����� */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* �Ĵ����� */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* ���� */
	
	if (value != 0 && value != 1)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* ����ֵ����� */
		goto err_ret;
	}
	
	if (reg == REG_D01)
	{
		g_tVar.D01 = value;
	}
	else if (reg == REG_D02)
	{
		g_tVar.D02 = value;
	}
	else if (reg == REG_D03)
	{
		g_tVar.D03 = value;
	}
	else if (reg == REG_D04)
	{
		g_tVar.D04 = value;
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* �Ĵ�����ַ���� */
	}
err_ret:
	if (g_tModS.RspCode == RSP_OK)				/* ��ȷӦ�� */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);		/* ��������������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_06H
*	����˵��: д�����Ĵ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_06H(void)
{

	/*
		д���ּĴ�����ע��06ָ��ֻ�ܲ����������ּĴ�����16ָ��������õ����������ּĴ���

		��������:
			11 �ӻ���ַ
			06 ������
			00 �Ĵ�����ַ���ֽ�
			01 �Ĵ�����ַ���ֽ�
			00 ����1���ֽ�
			01 ����1���ֽ�
			9A CRCУ����ֽ�
			9B CRCУ����ֽ�

		�ӻ���Ӧ:
			11 �ӻ���ַ
			06 ������
			00 �Ĵ�����ַ���ֽ�
			01 �Ĵ�����ַ���ֽ�
			00 ����1���ֽ�
			01 ����1���ֽ�
			1B CRCУ����ֽ�
			5A	CRCУ����ֽ�

		����:
			01 06 30 06 00 25  A710    ---- ������������Ϊ 2.5
			01 06 30 06 00 10  6707    ---- ������������Ϊ 1.0


			01 06 30 1B 00 00  F6CD    ---- SMA �˲�ϵ�� = 0 �ر��˲�
			01 06 30 1B 00 01  370D    ---- SMA �˲�ϵ�� = 1
			01 06 30 1B 00 02  770C    ---- SMA �˲�ϵ�� = 2
			01 06 30 1B 00 05  36CE    ---- SMA �˲�ϵ�� = 5

			01 06 30 07 00 01  F6CB    ---- ����ģʽ�޸�Ϊ T1
			01 06 30 07 00 02  B6CA    ---- ����ģʽ�޸�Ϊ T2

			01 06 31 00 00 00  8736    ---- ������ӿ��¼��
			01 06 31 01 00 00  D6F6    ---- �����澯��¼��

*/

	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* ����ֵ����� */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* �Ĵ����� */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* �Ĵ���ֵ */

	if (MODS_WriteRegValue(reg, value) == 1)	/* �ú������д���ֵ����Ĵ��� */
	{
		;
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* �Ĵ�����ַ���� */
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK)				/* ��ȷӦ�� */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);		/* ��������������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_10H
*	����˵��: ����д����Ĵ���.  �����ڸ�дʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_10H(void)
{
	/*
		�ӻ���ַΪ11H�����ּĴ�������ʵ��ַΪ0001H���Ĵ����Ľ�����ַΪ0002H���ܹ�����2���Ĵ�����
		���ּĴ���0001H������Ϊ000AH�����ּĴ���0002H������Ϊ0102H��

		��������:
			11 �ӻ���ַ
			10 ������
			00 �Ĵ�����ʼ��ַ���ֽ�
			01 �Ĵ�����ʼ��ַ���ֽ�
			00 �Ĵ����������ֽ�
			02 �Ĵ����������ֽ�
			04 �ֽ���
			00 ����1���ֽ�
			0A ����1���ֽ�
			01 ����2���ֽ�
			02 ����2���ֽ�
			C6 CRCУ����ֽ�
			F0 CRCУ����ֽ�

		�ӻ���Ӧ:
			11 �ӻ���ַ
			06 ������
			00 �Ĵ�����ַ���ֽ�
			01 �Ĵ�����ַ���ֽ�
			00 ����1���ֽ�
			01 ����1���ֽ�
			1B CRCУ����ֽ�
			5A	CRCУ����ֽ�

		����:
			01 10 30 00 00 06 0C  07 DE  00 0A  00 01  00 08  00 0C  00 00     389A    ---- дʱ�� 2014-10-01 08:12:00
			01 10 30 00 00 06 0C  07 DF  00 01  00 1F  00 17  00 3B  00 39     5549    ---- дʱ�� 2015-01-31 23:59:57

	*/
	uint16_t reg_addr;
	uint16_t reg_num;
	uint8_t byte_num;
	uint8_t i;
	uint16_t value;
	
	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount < 11)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;			/* ����ֵ����� */
		goto err_ret;
	}

	reg_addr = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* �Ĵ����� */
	reg_num = BEBufToUint16(&g_tModS.RxBuf[4]);		/* �Ĵ������� */
	byte_num = g_tModS.RxBuf[6];					/* ������������ֽ��� */

	if (byte_num != 2 * reg_num)
	{
		;
	}
	
	for (i = 0; i < reg_num; i++)
	{
		value = BEBufToUint16(&g_tModS.RxBuf[7 + 2 * i]);	/* �Ĵ���ֵ */

		if (MODS_WriteRegValue(reg_addr + i, value) == 1)
		{
			;
		}
		else
		{
			g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* �Ĵ�����ַ���� */
			break;
		}
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK)					/* ��ȷӦ�� */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);			/* ��������������� */
	}
}

 
