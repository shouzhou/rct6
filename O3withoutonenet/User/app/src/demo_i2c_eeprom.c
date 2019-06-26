/*
*********************************************************************************************************
*
*	模块名称 : EEPROM读写演示模块。
*	文件名称 : eeDemo.c
*	版    本 : V1.1
*	说    明 : EERPOM （24xx）读写例程。安富莱STM32-F4开发板标配的EEPROM型号为 AT24C128  (16K字节,128Kbit)
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*		V1.1    2013-06-20 armfly  更换读取串口命令的写法，不采用 getchar() 阻塞方式。
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* 仅允许本文件内调用的函数声明 */
static void ee_DispMenu(void);
static void ee_ReadTest(void);
static void ee_WriteTest(void);
static void ee_Erase(void);

uint8_t buf[EE_SIZE];	/* 数据测试用缓冲区 16KB */

/*
*********************************************************************************************************
*	函 数 名: DemoEEPROM
*	功能说明: 串行EEPROM读写例程
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoEEPROM(void)
{
	uint8_t cmd;
    uint8_t flag =0;

	if (ee_CheckOk() == 0)
	{
		/* 没有检测到EEPROM */
		printf("没有检测到串行EEPROM!\r\n");

		while (1);	/* 停机 */
	}

	printf("已经检测到串行EEPROM : \r\n");
	printf("型号: %s, 容量 = %d 字节, 页面大小 = %d\r\n", EE_MODEL_NAME, EE_SIZE, EE_PAGE_SIZE);

	ee_DispMenu();		/* 打印命令提示 */
	while(1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
        if(flag ) break;
		
		//cmd = getchar();	/* 从串口读入一个字符 (阻塞方式) */
		if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
		{
			switch (cmd)
			{
				case '1':
					printf("\r\n【1 - 读 EEPROM 测试】\r\n");
					ee_ReadTest();		/* 读EEPROM数据，并打印出来数据内容 */
					break;

				case '2':
					printf("\r\n【2 - 写 EEPROM 测试】\r\n");
					ee_WriteTest();		/* 写EEPROM数据，并打印写入速度 */
					break;

				case '3':
					printf("\r\n【3 - 擦除 EEPROM】\r\n");
					ee_Erase();			/* 擦差EEPROM数据，实际上就是写入全0xFF */
					break;
                
                case 'x':
					printf("\r\n【4 - 退出Demo】\r\n");
					flag = 1;
					break;

				default:
					ee_DispMenu();	/* 无效命令，重新打印命令提示 */
					break;

			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadTest
*	功能说明: 读串行EEPROM全部数据，并打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ee_ReadTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;

	/* 读EEPROM, 起始地址 = 0， 数据长度为 256 */
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	if (ee_ReadBytes((uint8_t *)buf, 0, EE_SIZE) == 0)
	{
		printf("读eeprom出错！\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
		printf("读eeprom成功，数据如下：\r\n");
	}

	/* 打印数据 */
	for (i = 0; i < EE_SIZE; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* 每行显示16字节数据 */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}

	/* 打印读速度 */
	printf("读耗时: %dms, 读速度: %dB/s\r\n", iTime2 - iTime1, (EE_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadTest
*	功能说明: 写串行EEPROM全部数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ee_WriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;

	/* 填充测试缓冲区 */
	for (i = 0; i < EE_SIZE; i++)
	{
		buf[i] = i;
	}

	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	if (ee_WriteBytes(0,buf, 0, EE_SIZE) == 0)
	{
		printf("写eeprom出错！\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
		printf("写eeprom成功！\r\n");
	}


	/* 打印读速度 */
	printf("写耗时: %dms, 写速度: %dB/s\r\n", iTime2 - iTime1, (EE_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadTest
*	功能说明: 读串行EEPROM全部数据，并打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ee_Erase(void)
{
	uint16_t i;

	/* 填充缓冲区 */
	for (i = 0; i < EE_SIZE; i++)
	{
		buf[i] = 0xFF;
	}

	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	if (ee_WriteBytes(0,buf, 0, EE_SIZE) == 0)
	{
		printf("擦除eeprom出错！\r\n");
		return;
	}
	else
	{
		printf("擦除eeprom成功！\r\n");
	}
}

/*
*********************************************************************************************************
*	函 数 名: ee_DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ee_DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择操作命令:\r\n");
	printf("1 - 读EEPROM (%d 字节)\r\n", EE_SIZE);
	printf("2 - 写EEPROM (%d 字节,0x00-0xFF)\r\n", EE_SIZE);
	printf("3 - 擦除EEPROM\r\n");
	printf("4 - 显示命令提示\r\n");
    printf("x - 退出Demo程序\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
