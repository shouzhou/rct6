/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.4
*	˵    �� : RS485 MODBUS��վ���̣�ʹ�õ��Ǵ���3����
*              ��������Ҫ����MODBUSЭ���վ���������,�����˳��õ����
*   ʵ�����ݣ�
*              1. �Ӻ�Ӳ��,(1)����1(��ӡʵ������)  (2)485�ӿ�(�շ�����)
*              2. ��������:
*								  ��	  �ӻ���ַ ������	�Ĵ����׵�ַ   �Ĵ�������	У����
*					KEY_DOWN_K1 : ���� 0x 	01 		 01		   01 01 		00 04 		6D F5
*					KEY_DOWN_K2	: ���� 0x   01       03        03 01        00 02       95 8F	
* 					JOY_DOWN_OK	: ���� 0x   01       02        02 01        00 03       68 73  				
*					JOY_UP_OK   : ���� 0x   01       04        04 01        00 01       61 3A
*								  д(1��) �ӻ���ַ ������    �Ĵ�����ַ	   д���ֵ		У����
*					JOY_DOWN_U	: ���� 0x   01       06        03 01        00 01       19 8E
*					JOY_DOWN_D	: ���� 0x   01       06        03 01        00 00       D8 4E
*					JOY_DOWN_L	: ���� 0x   01       05        01 01        00 01       5C 36
*					JOY_DOWN_R	: ���� 0x   01       05        01 01        00 00       9D F6
*								  д(���)�ӻ���ַ ������    �Ĵ�����ַ    �Ĵ�������  �ֽ���   д���ֵ1   д���ֵ2   У����
*					KEY_DOWN_K3 : ���� 0x   01       10        03 01        00 02        04      00 01       02 03      36 32
*	�޸ļ�¼ :
*		�汾��   ����       ����       ˵��
*		v1.0    2016-01-11  armfly     �׷�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"			/* �ײ�Ӳ������ */
#include "main.h"
#include "modbus_host.h"

/* ���������������̷������� */
#define EXAMPLE_NAME	"V4-RS485 MODBUS��վ����"
#define EXAMPLE_DATE	"2016-01-11"
#define DEMO_VER		"1.0"

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void DispMenu(void);

PRINT_MODS_T g_tPrint;
/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t testvalue =0x01;
    uint8_t ucKeyCode;				/* �������� */
	MSG_T ucMsg;					/* ��Ϣ���� */

	bsp_Init();						/* Ӳ����ʼ�� */
	PrintfLogo();					/* ��ӡ������Ϣ������1 */
	DispMenu();						/* ��ӡ�Ĵ�����ֵ */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();					/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		
		if (bsp_GetMsg(&ucMsg))		/* ��ȡ��Ϣ���� */
		{
			switch (ucMsg.MsgCode)
			{
				case MSG_MODS:		
					DispMenu();		/* ��ӡʵ���� */
					break;
				
				default:
					break;
			}
		}
	
		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
            testvalue= testvalue + 0x01;
			bsp_PutMsg(MSG_MODS, 0);
			
			switch (ucKeyCode)
			{			
				case KEY_DOWN_K1:				/* K1������ */
					//if (MODH_ReadParam_01H(REG_D01, 4) == 1) ;else ;
                    {
						uint8_t buf[16];
						
						buf[0] = testvalue;
						buf[1] = testvalue;
						buf[2] = testvalue;
						buf[3] = testvalue;
                        buf[4] = testvalue;
						buf[5] = testvalue;
						buf[6] = testvalue;
						buf[7] = testvalue;
                        buf[8] = testvalue;
						buf[9] = testvalue;
						buf[10] = testvalue;
						buf[11] = testvalue;
                        buf[12] = testvalue;
						buf[13] = testvalue;
						buf[14] = testvalue;
						buf[15] = testvalue;
						if (MODH_WriteParam_10H(REG_P01, 8, buf) == 1) ;else ;
					}
                    printf("k1 pressed \r\n");
					break;
				
				case KEY_DOWN_K2:				/* K2������ */
					if (MODH_ReadParam_03H(REG_P01, 8) == 1) ;else ;
                    printf("k2 pressed \r\n");
					break;
				
//				case KEY_DOWN_K3:				/* K3������ */
//					{
//						uint8_t buf[4];
//						
//						buf[0] = 0x01;
//						buf[1] = 0x02;
//						buf[2] = 0x03;
//						buf[3] = 0x04;
//						if (MODH_WriteParam_10H(REG_P01, 2, buf) == 1) ;else ;
//					}
//					break;
//				
//				case JOY_DOWN_U:				/* ҡ��UP������ */
//					if (MODH_WriteParam_06H(REG_P01, 1) == 1) ;else ;
//					break;
//				
//				case JOY_DOWN_D:				/* ҡ��DOWN������ */
//					if (MODH_WriteParam_06H(REG_P01, 0) == 1) ;else ;
//					break;
//				
//				case JOY_DOWN_L:				/* ҡ��LEFT������ */
//					if (MODH_WriteParam_05H(REG_D01, 1) == 1) ;else ;
//					break;
//				
//				case JOY_DOWN_R:				/* ҡ��RIGHT������ */
//					if (MODH_WriteParam_05H(REG_D01, 0) == 1) ;else ;
//					break;
//				
//				case JOY_DOWN_OK:				/* ҡ��OK������ */
//					if (MODH_ReadParam_02H(REG_T01, 3) == 1) ;else ;
//					break;

//				case JOY_UP_OK:					/* ҡ��OK������ */
//					if (MODH_ReadParam_04H(REG_A01, 1) == 1) ;else ;	
//					break;
				
				default:
					/* �����ļ�ֵ������ */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST�̼���汾����3���������stm32f10x.h�ļ��� */
	printf("* �̼���汾 : V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("* \n\r");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* �Ա���: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}
/*
*********************************************************************************************************
*	�� �� ��: DispMenu
*	����˵��: ��ӡ���̽��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{	
	uint8_t i;
	
	printf("\n\r");
	//printf("\33[K");						/* ����ӹ�굽��β������ */ 
	
	printf(" ���͵����� : 0x");				/* ��ӡ�������� */
	for (i = 0; i < g_tPrint.Txlen; i++)
	{
		printf(" %02X", g_tPrint.TxBuf[i]);
	}
    g_tPrint.Txlen =0;
	printf("\n\r");
	//printf("\33[K");						/* ����ӹ�굽��β������ */ 
	
	printf(" ���յ����� : 0x");				/* ��ӡ�������� */
	for (i = 0; i < g_tPrint.Rxlen; i++)
	{
		printf(" %02X", g_tPrint.RxBuf[i]);
	}
	g_tPrint.Rxlen =0;
	printf("\n\r");
	//printf("\33[3A");						/* �������3�� */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
