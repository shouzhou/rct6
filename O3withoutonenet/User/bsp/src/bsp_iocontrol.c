#include "bsp.h"

/* 按键口对应的RCC时钟 */
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
*	函 数 名: bsp_InitIO
*	功能说明: 配置IO指示灯相关的GPIO,  该函数被 bsp_Init() 调用。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_ALL_IO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	/*
		配置所有的IO指示灯GPIO为推挽输出模式
		由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动IO点亮.
		这是我不希望的，因此在改变GPIO为输出前，先关闭IO指示灯
	*/
//	bsp_IOOn(1);
//	bsp_IOOff(2);
//	bsp_IOOff(3);
//	bsp_IOOn(4);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* 推挽输出模式 */
	
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
*	函 数 名: bsp_IOOn
*	功能说明: 点亮指定的IO指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 无
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
*	函 数 名: bsp_IOOff
*	功能说明: 熄灭指定的IO指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 无
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
*	函 数 名: bsp_IOToggle
*	功能说明: 翻转指定的IO指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
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
*	函 数 名: bsp_IsIOOn
*	功能说明: 判断IO指示灯是否已经点亮。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 1表示已经点亮，0表示未点亮
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

 
