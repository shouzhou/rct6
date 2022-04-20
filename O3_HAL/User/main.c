
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V-跑马灯"
#define EXAMPLE_DATE	"2021-11-1"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
//    uint16_t count =0;
    uint8_t ucKeyCode;	
	uint8_t read,read2,read3;
	const char buf1[] = "接收到串口命令1\r\n";
	const char buf2[] = "接收到串口命令2\r\n";
	const char buf3[] = "接收到串口命令3\r\n";
	const char buf4[] = "接收到串口命令4\r\n";
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	/* 先做个LED1的亮灭显示 */
	bsp_LedOn(1);
	bsp_DelayMS(100);
	bsp_LedOff(1);
	bsp_DelayMS(100);
	
	bsp_StartAutoTimer(0, 500); /* 启动1个100ms的自动重装的定时器 */
	bsp_StartAutoTimer(1, 10000);	/* 启动1个500ms的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
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
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_LONG_K1:		/* 按键K1键按下 */
					printf("按键K1长按\r\n");
					bsp_LedToggle(1);	
					break;		
                case KEY_DOWN_K1:		/* 按键K1键按下 */
					printf("按键K1按下\r\n");
					bsp_LedToggle(1);	
					break;
				
				case KEY_DOWN_K2:		/* 按键K2键按下 */
					printf("按键K2按下\r\n");
					bsp_LedToggle(3);					
					break;

				case SYS_DOWN_K1K2:		/* 按键K3键按下 */
					printf("组合K1K2按下\r\n");	
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
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED1和LED2\r\n");
	printf("2. 再启动一个自动重装软件定时器，每500ms翻转一次LED3和LED4\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
//	/* 检测CPU ID */
//	{
//		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
//		
//		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
//		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
//		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

//		printf("\r\nCPU : STM32F429BIT6, LQFP208, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
//		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
//	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
