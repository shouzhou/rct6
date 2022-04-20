
#include "bsp.h"			/* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V-�����"
#define EXAMPLE_DATE	"2021-11-1"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
//    uint16_t count =0;
    uint8_t ucKeyCode;	
	uint8_t read,read2,read3;
	const char buf1[] = "���յ���������1\r\n";
	const char buf2[] = "���յ���������2\r\n";
	const char buf3[] = "���յ���������3\r\n";
	const char buf4[] = "���յ���������4\r\n";
	bsp_Init();		/* Ӳ����ʼ�� */
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	/* ������LED1��������ʾ */
	bsp_LedOn(1);
	bsp_DelayMS(100);
	bsp_LedOff(1);
	bsp_DelayMS(100);
	
	bsp_StartAutoTimer(0, 500); /* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	bsp_StartAutoTimer(1, 10000);	/* ����1��500ms���Զ���װ�Ķ�ʱ�� */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(1);			
		}
        
        if (comGetChar(COM1, &read))
		{
			switch (read)
			{
				case '1':
					comSendBuf(COM1, (uint8_t *)buf1, strlen(buf1));
					break;

				case '2':
					comSendBuf(COM1, (uint8_t *)buf2, strlen(buf2));
					break;

				case '3':
					comSendBuf(COM1, (uint8_t *)buf3, strlen(buf3));
					break;

				case '4':
					comSendBuf(COM1, (uint8_t *)buf4, strlen(buf4));
					break;	
				
				default:
					break;
			}
		}
        
        
        if (comGetChar(COM3, &read2))
		{
			switch (read2)
			{
				case '1':
					comSendBuf(COM3, (uint8_t *)buf1, strlen(buf1));
					break;

				case '2':
					comSendBuf(COM3, (uint8_t *)buf2, strlen(buf2));
					break;

				case '3':
					comSendBuf(COM3, (uint8_t *)buf3, strlen(buf3));
					break;

				case '4':
					comSendBuf(COM3, (uint8_t *)buf4, strlen(buf4));
					break;	
				
				default:
					break;
			}
		}
        
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case KEY_LONG_K1:		/* ����K1������ */
					printf("����K1����\r\n");
					bsp_LedToggle(1);	
					break;		
                case KEY_DOWN_K1:		/* ����K1������ */
					printf("����K1����\r\n");
					bsp_LedToggle(1);	
					break;
				
				case KEY_DOWN_K2:		/* ����K2������ */
					printf("����K2����\r\n");
					bsp_LedToggle(3);					
					break;

				case SYS_DOWN_K1K2:		/* ����K3������ */
					printf("���K1K2����\r\n");	
					bsp_LedToggle(4);	
					break;				
				
				default:
					break;
			}
		}
    }
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED1��LED2\r\n");
	printf("2. ������һ���Զ���װ�����ʱ����ÿ500ms��תһ��LED3��LED4\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
//	/* ���CPU ID */
//	{
//		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
//		
//		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
//		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
//		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

//		printf("\r\nCPU : STM32F429BIT6, LQFP208, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
//		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
//	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
