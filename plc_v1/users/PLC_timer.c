/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include <stdio.h> 
#include "PLC_Dialogue.h"
#include "PLC_CONF.H"

extern void filter(void);	                                //AD转换滤波
extern u16 PLC_16BIT[12100];	 //
extern u8 p_PLC_16BIT[];		   //
extern u16 After_filter[],phase;                          //ad转换数据缓冲器
extern void X_filter(void);                               //输入X信号滤波1MS一次
extern void DAC_data(void);




u16 PLC_RUN_TIME; //扫描时间
u16 temp[5];
u16 *p_data_given,*p_value;
extern bool receive_flag;
extern u8 Rx_header;
/*******************************************************************************
* 函数名  : Delay
* 描述    : 延时函数(ms)
* 输入    : d:延时系数，单位为毫秒
* 输出    : 无
* 返回    : 无 
* 说明    : 延时是利用Timer2定时器产生的1毫秒的计数来实现的
*******************************************************************************/
void Delay(u16 time)
{    
   u16 i=0;  
   while(time--)
   {i=12000; while(i--) ; }
}


void timer_enable(u16 timer_number)
{     
    p_data_given=PLC_16BIT+0x0900+timer_number;		 //计数器值地址
	  p_value=PLC_16BIT+0x0800+timer_number;			   //
		if(*p_value<*p_data_given)
		{
			if(PLC_BIT_TEST(0x600+timer_number)) 
			PLC_BIT_ON(0x3800+timer_number);             //复位线圈
		  else 
			{
				PLC_BIT_OFF(0x600+timer_number);           //值小于设定值时OFF
				if(PLC_BIT_TEST(0x3800+timer_number))
				{
				   *p_value=0;PLC_BIT_OFF(0x3800+timer_number);
				}
			}
		}
		else
	  PLC_BIT_ON(0x600+timer_number);                //值到达设定值时ON
}

void timer_disble(u16 timer_number)
{     
	p_data_given=PLC_16BIT+0x0900+timer_number;		//计数器值地址
	p_value=PLC_16BIT+0x0800+timer_number;			    //
	PLC_BIT_OFF(0x600+timer_number);	              //溢出线圈
	PLC_BIT_OFF(0x1600+timer_number);              //使能线圈
	PLC_BIT_OFF(0x3800+timer_number);              //复位线圈
	*p_data_given=0;
	*p_value=0;
}

void T_100MS(void)
{
	u16 timer_count;
  for(timer_count=0;timer_count<200;timer_count++)
	{ 
		  p_data_given=PLC_16BIT+0x0900+timer_count;
	    p_value=PLC_16BIT+0x0800+timer_count;
	    if(PLC_BIT_TEST(0x1600+timer_count))//线圈状态
		  {
			  if(*p_value<*p_data_given)					//值状态
				{
				  if(PLC_BIT_TEST(0x600+timer_count)) ;
				  else *p_value+=1;
				}
		  }
	 }
}

void T_10MS(void)
{  
   u16 timer_count;
   for(timer_count=200;timer_count<246;timer_count++)
   { 
		p_data_given=PLC_16BIT+0x0900+timer_count;
	    p_value=PLC_16BIT+0x0800+timer_count;
	    if(PLC_BIT_TEST(0x1600+timer_count)) //线圈状态
		  {
          if(*p_value<*p_data_given)					//值状态
				  {
				     if(PLC_BIT_TEST(0x600+timer_count)) ;
				     else *p_value+=1;
				  }
		  }
   }
}

void T_1MS(void)
{ 
	 u16 timer_count;  
     for(timer_count=246;timer_count<250;timer_count++)
	   { 
		  p_data_given=PLC_16BIT+0x0900+timer_count;
	      p_value=PLC_16BIT+0x0800+timer_count;
	      if(PLC_BIT_TEST(0x1600+timer_count))//线圈状态
		    {  
           if(*p_value<*p_data_given)					//值状态
				   {
				     if(PLC_BIT_TEST(0x600+timer_count)) ;
				     else *p_value+=1;
				  }
		    }		
	  }	
	  
}

void T_H100MS(void)
{ 
	  u16 timer_count;
    for(timer_count=250;timer_count<256;timer_count++)
	  { 
		 p_data_given=PLC_16BIT+0x0900+timer_count;
	     p_value=PLC_16BIT+0x0800+timer_count;
	     if(PLC_BIT_TEST(0x1600+timer_count))  //线圈状态
		   {
           if(*p_value<*p_data_given)				 //值状态
				   {
				     if(PLC_BIT_TEST(0x600+timer_count)) ;
				     else *p_value+=1;
				   }
		   }
	   }
}




void TIM2_IRQHandler(void)//1ms产生一次中断信号
{ 
	static u8 all_clock;
	static u16 minute;
	TIM2->SR=0; 	
	PLC_RUN_TIME+=10;
	all_clock++;
	X_filter();                     //检查X输入状态值			
	if(all_clock>99) 	              //m8011 10MS m8012 100MS  m8013 1SEC m8014 1minute   
	{
		all_clock=0,PLC_BIT_ON(M8012);T_1MS();
	}	             
	if((all_clock%10)==7)           //10ms定时器设计每计五次刷新一次
	{
		T_10MS(),PLC_BIT_OFF(M8011);
	}
	if((all_clock%10)==2)
	{
		PLC_BIT_ON(M8011);
	}
	if(all_clock==50)	              //两种100MS定时器分开刷新
	T_100MS(),PLC_BIT_OFF(M8012);	

	if(all_clock==90)	              //每100ms秒钟分钟定时器
	{
		T_H100MS(),minute++;
		receive_flag=FALSE;  //ADD
		Rx_header=FALSE;
	}
		
	if((all_clock%0x10)==0x02)	    //更新一次DAC数据
	{
		DAC_data();
		filter();                                            //ADC十个毫秒传送更新一次
	}

	if(minute%10==5)	              //刷新秒钟8013
	PLC_BIT_OFF(M8013);

	if(minute%10==0)
	PLC_BIT_ON(M8013);

	if(minute==300)			            //刷新分钟8014
	PLC_BIT_OFF(M8014);

	if(minute==0)
	PLC_BIT_ON(M8014);

	if(minute>599)
	minute=0;		  
}

void PLC_Timer(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;       
	/* TIM2 clock enable */	
	/* ---------------------------------------------------------------
	TIM2配置:输出比较时间模式:
	TIM2CLK = 36 MHz,预定标器= 7200,TIM5计数器时钟= 7.2 MHz
	--------------------------------------------------------------- */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	//这个就是自动装载的计数值，由于计数是从0开始的，计数72次后为71
	TIM_TimeBaseStructure.TIM_Period = 36000;
	// 这个就是预分频系数，当由于为0时表示不分频所以要减1
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	// 使用的采样频率之间的分频比例
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//向上计数
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//初始化定时器2
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* Clear TIM2 update pending flag[清除TIM5溢出中断标志] */
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	/* TIM IT enable */ //打开溢出中断
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);  //计数器使能，开始工作
}




