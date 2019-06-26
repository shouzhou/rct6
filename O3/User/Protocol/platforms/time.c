/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <time.h>
//#include <unistd.h>
//#include <sys/time.h>
#include "stm32f10x_rtc.h"
#include "stm32f10x.h"
#include "core_cm3.h"
static __IO time_t SystickTime=0; 
static u8  c_us=0;//us延时倍乘数
static u16 c_ms=0;//ms延时倍乘数



#define USE_TIM2
//#define USE_TIM3
//#define USE_TIM4
//#define USE_TIM5

#ifdef USE_TIM2
	#define TIM_HARD		TIM2
	#define TIM_HARD_IRQn	TIM2_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM2
#endif

#ifdef USE_TIM3
	#define TIM_HARD		TIM3
	#define TIM_HARD_IRQn	TIM3_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM3
#endif

#ifdef USE_TIM4
	#define TIM_HARD		TIM4
	#define TIM_HARD_IRQn	TIM4_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM4
#endif

#ifdef USE_TIM5
	#define TIM_HARD		TIM5
	#define TIM_HARD_IRQn	TIM5_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM5
#endif

/* 保存 TIM定时中断到后执行的回调函数指针 */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

void delay_init()	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	c_us=SystemCoreClock/8000000;	//为系统时钟的1/8  
	c_ms=(u16)c_us*1000;//非ucos下,代表每个ms需要的systick时钟数   
}

void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*c_us; 
	SysTick->VAL=0x00;       
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;     
	SysTick->VAL =0X00;     
}
void uDelay(uint32_t nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*c_us; 
	SysTick->VAL=0x00;       
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;     
	SysTick->VAL =0X00;     
}
/*
************************************************************
*	函数名称：	delay_ms_sub
*
*	函数功能：	微秒级长延时
*
*	入口参数：	delay_ms_sub：对72M条件下,delay_ms_sub<=1864 
*
*	返回参数：	无
************************************************************
*/
void delay_ms_sub(uint16_t sub_ms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)sub_ms*c_ms;
	SysTick->VAL =0x00;        
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;         
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;    
	SysTick->VAL =0X00;     
} 


/*
************************************************************
*	函数名称：	delay_ms
*
*	函数功能：	微秒级长延时
*
*	入口参数：	ms：延时的时长
*
*	返回参数：	无
************************************************************
*/
void mDelay(unsigned short ms)
{

	unsigned char repeat = 0;
	unsigned short remain = 0;
	
	repeat = ms / 500;
	remain = ms % 500;
	
	while(repeat)
	{
		delay_ms_sub(500);
		repeat--;
	}
	
	if(remain)
	{
		delay_ms_sub(remain);
	}
}


void RTC_Init(void)
{
	unsigned char temp = 0;
	RCC->APB1ENR|=1<<28;

	RCC->APB1ENR|=1<<27;

	PWR->CR|=1<<8;

	RCC->BDCR|=1<<16;

	RCC->BDCR&=~(1<<16); 

		//internal 40k;
	RCC->CSR|=(1<<0);
	while((!(RCC->CSR&0X02))&&temp<250)//等待外部时钟就绪 
	{ 
		mDelay(10);
		temp++;
	};
	
	if(temp>=250)
		return;
	RCC->BDCR&=~(0x3<<8);
	RCC->BDCR|=1<<9; 
	RCC->BDCR|=1<<15;
	while(!(RTC->CRL&(1<<5)));
	while(!(RTC->CRL&(1<<3)));
	RTC->CRH|=0X01;
	while(!(RTC->CRL&(1<<5)));
	RTC->CRL|=1<<4;
	
	RTC->PRLH=0X0000; 
	RTC->PRLL=39999; 
	RTC->CRL&=~(1<<4); 
	while(!(RTC->CRL&(1<<5)));
}

void nbiot_sleep( int milliseconds)
{
    mDelay(milliseconds);
	  return;
}


time_t nbiot_time( void )
{

   return (RTC->CNTL | RTC->CNTH << 16);
}

void nbiot_time_init(void)
{
    delay_init();
    RTC_Init();
    return ;
}


/*
*********************************************************************************************************
*	函 数 名: bsp_InitHardTimer
*	功能说明: 配置 TIMx，用于us级别硬件定时。TIMx将自由运行，永不停止.
*			TIMx可以用TIM2 - TIM5 之间的TIM, 这些TIM有4个通道, 挂在 APB1 上，输入时钟=SystemCoreClock / 2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if defined (USE_TIM2) || defined (USE_TIM3)  || defined (USE_TIM4)	|| defined (USE_TIM5)
void bsp_InitHardTimer(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

  	/* 使能TIM时钟 */
	RCC_APB1PeriphClockCmd(TIM_HARD_RCC, ENABLE);

    /*-----------------------------------------------------------------------
		system_stm32f4xx.c 文件中 void SetSysClock(void) 函数对时钟的配置如下：

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
		因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = PCLK2 x 2 = SystemCoreClock;

		APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13,TIM14
		APB2 定时器有 TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	uiTIMxCLK = SystemCoreClock / 2;

	usPrescaler = uiTIMxCLK / 1000000 ;	/* 分频到周期 1us */
	
#if defined (USE_TIM2) || defined (USE_TIM5) 
	//usPeriod = 0xFFFFFFFF;	/* 407支持32位定时器 */
	usPeriod = 0xFFFF;	/* 103支持16位 */
#else
	usPeriod = 0xFFFF;
#endif
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM_HARD, &TIM_TimeBaseStructure);

	//TIM_ARRPreloadConfig(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Cmd(TIM_HARD, ENABLE);

	/* 配置TIM定时中断 (Update) */
	{
		NVIC_InitTypeDef NVIC_InitStructure;	/* 中断结构体在 misc.h 中定义 */

		NVIC_InitStructure.NVIC_IRQChannel = TIM_HARD_IRQn;

		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	/* 比串口优先级低 */
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StartHardTimer
*	功能说明: 使用TIM2-5做单次定时器使用, 定时时间到后执行回调函数。可以同时启动4个定时器，互不干扰。
*             定时精度正负10us （主要耗费在调用本函数的执行时间，函数内部进行了补偿减小误差）
*			 TIM2和TIM5 是16位定时器。
*			 TIM3和TIM4 是16位定时器。
*	形    参: _CC : 捕获通道几，1，2，3, 4
*             _uiTimeOut : 超时时间, 单位 1us.       对于16位定时器，最大 65.5ms; 对于32位定时器，最大 4294秒
*             _pCallBack : 定时时间到后，被执行的函数
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;

    /*
        执行下面这个语句，时长 = 18us (通过逻辑分析仪测量IO翻转)
        bsp_StartTimer2(3, 500, (void *)test1);
    */
    if (_uiTimeOut < 5)
    {
        ;
    }
    else
    {
        _uiTimeOut -= 5;
    }

    cnt_now = TIM_GetCounter(TIM_HARD);    	/* 读取当前的计数器值 */
    cnt_tar = cnt_now + _uiTimeOut;			/* 计算捕获的计数器值 */
    if (_CC == 1)
    {
        s_TIM_CallBack1 = (void (*)(void))_pCallBack;

        TIM_SetCompare1(TIM_HARD, cnt_tar);      	/* 设置捕获比较计数器CC1 */
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC1);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC1, ENABLE);	/* 使能CC1中断 */

    }
    else if (_CC == 2)
    {
		s_TIM_CallBack2 = (void (*)(void))_pCallBack;

        TIM_SetCompare2(TIM_HARD, cnt_tar);      	/* 设置捕获比较计数器CC2 */
		TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC2);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC2, ENABLE);	/* 使能CC2中断 */
    }
    else if (_CC == 3)
    {
        s_TIM_CallBack3 = (void (*)(void))_pCallBack;

        TIM_SetCompare3(TIM_HARD, cnt_tar);      	/* 设置捕获比较计数器CC3 */
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC3);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC3, ENABLE);	/* 使能CC3中断 */
    }
    else if (_CC == 4)
    {
        s_TIM_CallBack4 = (void (*)(void))_pCallBack;

        TIM_SetCompare4(TIM_HARD, cnt_tar);      	/* 设置捕获比较计数器CC4 */
		TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC4);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC4, ENABLE);	/* 使能CC4中断 */
    }
	else
    {
        return;
    }
}
#endif

/*
*********************************************************************************************************
*	函 数 名: TIMx_IRQHandler
*	功能说明: TIM 中断服务程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

#ifdef USE_TIM2
void TIM2_IRQHandler(void)
#endif

#ifdef USE_TIM3
void TIM3_IRQHandler(void)
#endif

#ifdef USE_TIM4
void TIM4_IRQHandler(void)
#endif

#ifdef USE_TIM5
void TIM5_IRQHandler(void)
#endif
{
    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC1))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC1);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC1, DISABLE);	/* 禁能CC1中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack1();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC2))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC2);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC2, DISABLE);	/* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack2();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC3))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC3);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC3, DISABLE);	/* 禁能CC3中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack3();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC4))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC4);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC4, DISABLE);	/* 禁能CC4中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack4();
    }
}


