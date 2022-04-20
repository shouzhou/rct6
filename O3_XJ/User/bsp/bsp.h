/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��
*	�ļ����� : bsp.h
*	˵    �� : ���ǵײ�����ģ�����е�h�ļ��Ļ����ļ��� Ӧ�ó���ֻ�� #include bsp.h ���ɣ�
*			  ����Ҫ#include ÿ��ģ��� h �ļ�
*
*	Copyright (C), 2013-2014,    
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_



/* ���� BSP �汾�� */
#define __STM32F1_BSP_VERSION		"1.1"

/* CPU����ʱִ�еĺ��� */
//#define CPU_IDLE()		bsp_Idle()

/* ����ȫ���жϵĺ� */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */

/* ���������ڵ��Խ׶��Ŵ� */
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



/* ͨ��ȡ��ע�ͻ������ע�͵ķ�ʽ�����Ƿ�����ײ�����ģ�� */
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


/* �ṩ������C�ļ����õĺ��� */
void bsp_Init(void);
void bsp_Idle(void);

#endif

 
