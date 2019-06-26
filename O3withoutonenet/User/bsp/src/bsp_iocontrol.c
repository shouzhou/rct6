#include "bsp.h"

/* �����ڶ�Ӧ��RCCʱ�� */
#define RCC_ALL_IO 	(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB)

#define GPIO_PORT_IO1   GPIOA
#define GPIO_PIN_IO1	GPIO_Pin_15

#define GPIO_PORT_IO2   GPIOB
#define GPIO_PIN_IO2	GPIO_Pin_0

#define GPIO_PORT_IO3   GPIOB
#define GPIO_PIN_IO3	GPIO_Pin_1

#define GPIO_PORT_IO4   GPIOB
#define GPIO_PIN_IO4	GPIO_Pin_3

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitIO
*	����˵��: ����IOָʾ����ص�GPIO,  �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_APB2PeriphClockCmd(RCC_ALL_IO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	/*
		�������е�IOָʾ��GPIOΪ�������ģʽ
		���ڽ�GPIO����Ϊ���ʱ��GPIO����Ĵ�����ֵȱʡ��0����˻�����IO����.
		�����Ҳ�ϣ���ģ�����ڸı�GPIOΪ���ǰ���ȹر�IOָʾ��
	*/
//	bsp_IOOn(1);
//	bsp_IOOff(2);
//	bsp_IOOff(3);
//	bsp_IOOn(4);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* �������ģʽ */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IO1;
	GPIO_Init(GPIO_PORT_IO1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IO2;
	GPIO_Init(GPIO_PORT_IO2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IO3;
	GPIO_Init(GPIO_PORT_IO3, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IO4;
	GPIO_Init(GPIO_PORT_IO4, &GPIO_InitStructure);
    
   	bsp_IOOn(1);
	bsp_IOOn(2);
	bsp_IOOn(3);
	bsp_IOOn(4);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IOOn
*	����˵��: ����ָ����IOָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_IOOff(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_IO1->BRR = GPIO_PIN_IO1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_IO2->BRR = GPIO_PIN_IO2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_IO3->BRR = GPIO_PIN_IO3;
	}
	else if (_no == 3)
	{
		GPIO_PORT_IO4->BRR = GPIO_PIN_IO4;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IOOff
*	����˵��: Ϩ��ָ����IOָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_IOOn(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_IO1->BSRR = GPIO_PIN_IO1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_IO2->BSRR = GPIO_PIN_IO2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_IO3->BSRR = GPIO_PIN_IO3;
	}
	else if (_no == 3)
	{
		GPIO_PORT_IO4->BSRR = GPIO_PIN_IO4;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IOToggle
*	����˵��: ��תָ����IOָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_IOToggle(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_IO1->ODR ^= GPIO_PIN_IO1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_IO2->ODR ^= GPIO_PIN_IO2;
	}
	else if (_no == 3)
	{
		GPIO_PORT_IO3->ODR ^= GPIO_PIN_IO3;
	}
	else if (_no == 4)
	{
		GPIO_PORT_IO4->ODR ^= GPIO_PIN_IO4;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IsIOOn
*	����˵��: �ж�IOָʾ���Ƿ��Ѿ�������
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: 1��ʾ�Ѿ�������0��ʾδ����
*********************************************************************************************************
*/
uint8_t bsp_IsIOOn(uint8_t _no)
{
	if (_no == 1)
	{
		if ((GPIO_PORT_IO1->ODR & GPIO_PIN_IO1) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 2)
	{
		if ((GPIO_PORT_IO2->ODR & GPIO_PIN_IO2) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 3)
	{
		if ((GPIO_PORT_IO3->ODR & GPIO_PIN_IO3) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 4)
	{
		if ((GPIO_PORT_IO4->ODR & GPIO_PIN_IO4) == 0)
		{
			return 1;
		}
		return 0;
	}

	return 0;
}

 
