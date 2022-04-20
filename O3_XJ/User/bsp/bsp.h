/*
*********************************************************************************************************
*
*	模块名称 : BSP模块
*	文件名称 : bsp.h
*	说    明 : 这是底层驱动模块所有的h文件的汇总文件。 应用程序只需 #include bsp.h 即可，
*			  不需要#include 每个模块的 h 文件
*
*	Copyright (C), 2013-2014,    
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_



/* 定义 BSP 版本号 */
#define __STM32F1_BSP_VERSION		"1.1"

/* CPU空闲时执行的函数 */
//#define CPU_IDLE()		bsp_Idle()

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf
//#define BSP_Printf(...)


#define  USEDEBUG

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif



/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
#include "bsp_uart_fifo.h"
//#include "bsp_usart1.h"
#include "bsp_usart3.h"
#include "bsp_led.h"
#include "bsp_timer.h"
#include "bsp_key.h"
#include "bsp_msg.h"
#include "bsp_diwen.h"
#include "bsp_dht22.h"
//#include "bsp_dht22_atk.h"
#include "bsp_i2c_gpio.h"
//#include "bsp_adc.h"
#include "bsp_hc.h"
#include "bsp_tim_capture.h"
#include "bsp_i2c_gpio.h"
#include "bsp_eeprom_24xx.h"
#include "bsp_cpu_rtc.h"
#include "bsp_adc.h"
//#include "demo_i2c_eeprom.h"
#include "bsp_iocontrol.h"
#include "bsp_iwdg.h"
//#include "bsp_mkey.h"
//#include "delay.h"

#include "func.h"
#include "set.h"


//#include "at_cmd.h"
//#include "utils.h"
//#include "M5310.h"
//#include "fifo.h"
//#include "platform.h"
//#include "ringbuf.h"
//#include "nbiot.h"


/* 提供给其他C文件调用的函数 */
void bsp_Init(void);
void bsp_Idle(void);

#endif

 
