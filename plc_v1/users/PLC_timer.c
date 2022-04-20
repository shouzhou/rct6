/********************************************************/
// CPU��Ҫ��STM32F103--RAM�ڴ治С��64K	Flash�ڴ治С��128K
// ����������STM32F103RDT6��VET6����ͨ��
// �༭���ڣ�20150909
// editor by СС��
// ���꣺shop182385147.taobao.com
/********************************************************/

#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include <stdio.h> 
#include "PLC_Dialogue.h"
#include "PLC_CONF.H"

extern void filter(void);	                                //ADת���˲�
extern u16 PLC_16BIT[12100];	 //
extern u8 p_PLC_16BIT[];		   //
extern u16 After_filter[],phase;                          //adת�����ݻ�����
extern void X_filter(void);                               //����X�ź��˲�1MSһ��
extern void DAC_data(void);




u16 PLC_RUN_TIME; //ɨ��ʱ��
u16 temp[5];
u16 *p_data_given,*p_value;
extern bool receive_flag;
extern u8 Rx_header;
/*******************************************************************************
* ������  : Delay
* ����    : ��ʱ����(ms)
* ����    : d:��ʱϵ������λΪ����
* ���    : ��
* ����    : �� 
* ˵��    : ��ʱ������Timer2��ʱ��������1����ļ�����ʵ�ֵ�
*******************************************************************************/
void Delay(u16 time)
{    
   u16 i=0;  
   while(time--)
   {i=12000; while(i--) ; }
}


void timer_enable(u16 timer_number)
{     
    p_data_given=PLC_16BIT+0x0900+timer_number;		 //������ֵ��ַ
	  p_value=PLC_16BIT+0x0800+timer_number;			   //
		if(*p_value<*p_data_given)
		{
			if(PLC_BIT_TEST(0x600+timer_number)) 
			PLC_BIT_ON(0x3800+timer_number);             //��λ��Ȧ
		  else 
			{
				PLC_BIT_OFF(0x600+timer_number);           //ֵС���趨ֵʱOFF
				if(PLC_BIT_TEST(0x3800+timer_number))
				{
				   *p_value=0;PLC_BIT_OFF(0x3800+timer_number);
				}
			}
		}
		else
	  PLC_BIT_ON(0x600+timer_number);                //ֵ�����趨ֵʱON
}

void timer_disble(u16 timer_number)
{     
	p_data_given=PLC_16BIT+0x0900+timer_number;		//������ֵ��ַ
	p_value=PLC_16BIT+0x0800+timer_number;			    //
	PLC_BIT_OFF(0x600+timer_number);	              //�����Ȧ
	PLC_BIT_OFF(0x1600+timer_number);              //ʹ����Ȧ
	PLC_BIT_OFF(0x3800+timer_number);              //��λ��Ȧ
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
	    if(PLC_BIT_TEST(0x1600+timer_count))//��Ȧ״̬
		  {
			  if(*p_value<*p_data_given)					//ֵ״̬
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
	    if(PLC_BIT_TEST(0x1600+timer_count)) //��Ȧ״̬
		  {
          if(*p_value<*p_data_given)					//ֵ״̬
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
	      if(PLC_BIT_TEST(0x1600+timer_count))//��Ȧ״̬
		    {  
           if(*p_value<*p_data_given)					//ֵ״̬
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
	     if(PLC_BIT_TEST(0x1600+timer_count))  //��Ȧ״̬
		   {
           if(*p_value<*p_data_given)				 //ֵ״̬
				   {
				     if(PLC_BIT_TEST(0x600+timer_count)) ;
				     else *p_value+=1;
				   }
		   }
	   }
}




void TIM2_IRQHandler(void)//1ms����һ���ж��ź�
{ 
	static u8 all_clock;
	static u16 minute;
	TIM2->SR=0; 	
	PLC_RUN_TIME+=10;
	all_clock++;
	X_filter();                     //���X����״ֵ̬			
	if(all_clock>99) 	              //m8011 10MS m8012 100MS  m8013 1SEC m8014 1minute   
	{
		all_clock=0,PLC_BIT_ON(M8012);T_1MS();
	}	             
	if((all_clock%10)==7)           //10ms��ʱ�����ÿ�����ˢ��һ��
	{
		T_10MS(),PLC_BIT_OFF(M8011);
	}
	if((all_clock%10)==2)
	{
		PLC_BIT_ON(M8011);
	}
	if(all_clock==50)	              //����100MS��ʱ���ֿ�ˢ��
	T_100MS(),PLC_BIT_OFF(M8012);	

	if(all_clock==90)	              //ÿ100ms���ӷ��Ӷ�ʱ��
	{
		T_H100MS(),minute++;
		receive_flag=FALSE;  //ADD
		Rx_header=FALSE;
	}
		
	if((all_clock%0x10)==0x02)	    //����һ��DAC����
	{
		DAC_data();
		filter();                                            //ADCʮ�����봫�͸���һ��
	}

	if(minute%10==5)	              //ˢ������8013
	PLC_BIT_OFF(M8013);

	if(minute%10==0)
	PLC_BIT_ON(M8013);

	if(minute==300)			            //ˢ�·���8014
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
	TIM2����:����Ƚ�ʱ��ģʽ:
	TIM2CLK = 36 MHz,Ԥ������= 7200,TIM5������ʱ��= 7.2 MHz
	--------------------------------------------------------------- */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	//��������Զ�װ�صļ���ֵ�����ڼ����Ǵ�0��ʼ�ģ�����72�κ�Ϊ71
	TIM_TimeBaseStructure.TIM_Period = 36000;
	// �������Ԥ��Ƶϵ����������Ϊ0ʱ��ʾ����Ƶ����Ҫ��1
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	// ʹ�õĲ���Ƶ��֮��ķ�Ƶ����
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//��ʼ����ʱ��2
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* Clear TIM2 update pending flag[���TIM5����жϱ�־] */
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	/* TIM IT enable */ //������ж�
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);  //������ʹ�ܣ���ʼ����
}




