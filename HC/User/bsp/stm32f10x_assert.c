/*
*********************************************************************************************************
*	                                  
*	模块名称 : 断言模块。
*	文件名称 : stm32f10x_assert.c
*	版    本 : V1.0
*	说    明 : 提供断言函数,主要用于程序调试。ST固件库中的函数均可以对输入参数进行检查，提高程序的健壮性。
*			   这个文件是 创建的文件，不属于标准库的文件，ST固件库的范例将这些函数放在main.c文件。
*			   我们认为这个和用户的具体应用无关，因此将其独立出来，使main.c文件看起来更加简洁一些。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-27 armfly  ST固件库V3.4.0版本。
*		v2.0    2011-10-16 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2010-2011,    
*
*********************************************************************************************************
*/

#include "stm32f10x.h"	/* 这个文件包含了stm32f10x_conf.h, stm32f10x_conf.h文件定义了USE_FULL_ASSERT */
#include <stdio.h>

/* 
	ST库函数使用了C编译器的断言功能，如果定义了USE_FULL_ASSERT，那么所有的ST库函数将检查函数形参
	是否正确。如果不正确将调用 assert_failed() 函数，这个函数是一个死循环，便于用户检查代码。
	
	关键字 __LINE__ 表示源代码行号。
	关键字__FILE__表示源代码文件名。
	
	断言功能使能后将增大代码大小，推荐用户仅在调试时使能，在正式发布软件是禁止。

	用户可以选择是否使能ST固件库的断言供能。使能断言的方法有两种：
	(1) 在C编译器的预定义宏选项中定义USE_FULL_ASSERT。
	(2) 在本文件取消"#define USE_FULL_ASSERT    1"行的注释。	
*/
#ifdef USE_FULL_ASSERT

/*
*********************************************************************************************************
*	函 数 名: assert_failed
*	形    参：file : 源代码文件名称。关键字__FILE__表示源代码文件名。
*			  line ：代码行号。关键字 __LINE__ 表示源代码行号
*	返 回 值: 无
*********************************************************************************************************
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* 
		用户可以添加自己的代码报告源代码文件名和代码行号，比如将错误文件和行号打印到串口
		printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	*/
	
	/* 这是一个死循环，断言失败时程序会在此处死机，以便于用户查错 */
	while (1)
	{
	}
}
#endif
