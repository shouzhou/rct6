#include "bsp.h"

// 如果需要TIM3重映射 ，打开该宏定义即可
#define  TIM3REMAP    

uint16_t count=0;
uint8_t  secondflag =0;
void  BASIC_TIM_IRQHandler (void)  //about  10ms  period
{
	if ( TIM_GetITStatus( BASIC_TIM, TIM_IT_Update) != RESET ) 
	{	
        count++;
 
        if(count ==100)
        {
            count =0;
            secondflag =1;
        }
    
 
		TIM_ClearITPendingBit(BASIC_TIM , TIM_FLAG_Update);  		 
	}		 	
}
// 中断优先级配置
static void BASIC_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
  //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
    NVIC_InitStructure.NVIC_IRQChannel = BASIC_TIM_IRQ ;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	 
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void BASIC_TIM_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    BASIC_TIM_APBxClock_FUN(BASIC_TIM_CLK, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = BASIC_TIM_Period;	
    TIM_TimeBaseStructure.TIM_Prescaler= BASIC_TIM_Prescaler;
    TIM_TimeBaseInit(BASIC_TIM, &TIM_TimeBaseStructure);
    TIM_ClearFlag(BASIC_TIM, TIM_FLAG_Update);
    TIM_ITConfig(BASIC_TIM,TIM_IT_Update,ENABLE);
    TIM_Cmd(BASIC_TIM, ENABLE);	
}

void BASIC_TIM_Init(void)
{
	BASIC_TIM_NVIC_Config();
	BASIC_TIM_Mode_Config();
}

TIM_ICInitTypeDef  TIM3_ICInitStructure;
///////////////////////TIM3原始的四通道 /////////////////////
//PA6 --TIM3CH1
//PA7 --TIM3CH2
//PB0 --TIM3CH3
//PB1 --TIM3CH4

///////////////////////TIM3重映射的四通道 /////////////////////
//PC6 --TIM3CH1
//PC7 --TIM3CH2
//PC8 --TIM3CH3
//PC9 --TIM3CH4


void TIM3_CAP_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;
 
	#ifndef  TIM3REMAP
    //原始配置
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能TIM3时钟
 
    #else 
  //重映射配置  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5  
    #endif
    
  
   #ifndef TIM3REMAP    
    // 原始配置
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	#else
//  REMAP 配置 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 输入  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   #endif

    
	//初始化定时器5 TIM3	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM3输入捕获参数
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_4; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  //先占优先级2级
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
   	TIM_Cmd(TIM3,ENABLE ); 	//使能定时器5
   


}

//u8  CAPTURE_STA_TIM3CH[0]=0;	//输入捕获状态		    				
//u16	CAPTURE_VAL_TIM3CH[0];	//输入捕获值
u8 CAPTURE_STA_TIM3CH[4] = {0};
u16 CAPTURE_VAL_TIM3CH[4];
u16 CAPTURE_UP_TIM3CH[4], CAPTURE_DOWN_TIM3CH[4];
//定时器5中断服务程序	 
void TIM3_IRQHandler(void)
{ 
   //通道一
 	if((CAPTURE_STA_TIM3CH[0]&0X80)==0)//还未成功捕获	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[0]&0X40)//已经捕获到高电平了
			{
				if((CAPTURE_STA_TIM3CH[0]&0X3F)==0X3F)//高电平太长了
				{
					CAPTURE_STA_TIM3CH[0]|=0X80;//标记成功捕获了一次
					CAPTURE_VAL_TIM3CH[0]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[0]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
		{
          
			if(CAPTURE_STA_TIM3CH[0]&0X40)		//捕获到一个下降沿 		
			{	  			
				CAPTURE_STA_TIM3CH[0] |= 0x80;                    //标记成功捕获到一次高电平脉宽
                CAPTURE_DOWN_TIM3CH[0] = TIM_GetCapture1(TIM3);
                if(CAPTURE_DOWN_TIM3CH[0] >= CAPTURE_UP_TIM3CH[0]) 
                    CAPTURE_VAL_TIM3CH[0] = CAPTURE_DOWN_TIM3CH[0] - CAPTURE_UP_TIM3CH[0];
                else 
                    CAPTURE_VAL_TIM3CH[0] = 0xffff + CAPTURE_DOWN_TIM3CH[0] - CAPTURE_UP_TIM3CH[0];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				CAPTURE_STA_TIM3CH[0]=0;			//清空
				CAPTURE_VAL_TIM3CH[0]=0;
	 			CAPTURE_UP_TIM3CH[0] = TIM_GetCapture1(TIM3);
				CAPTURE_STA_TIM3CH[0]|=0X40;		//标记捕获到了上升沿
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
    //通道二  
   if((CAPTURE_STA_TIM3CH[1]&0X80)==0)//还未成功捕获	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[1]&0X40)//已经捕获到高电平了
			{
				if((CAPTURE_STA_TIM3CH[1]&0X3F)==0X3F)//高电平太长了
				{
					CAPTURE_STA_TIM3CH[1]|=0X80;//标记成功捕获了一次
					CAPTURE_VAL_TIM3CH[1]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[1]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)//捕获1发生捕获事件
		{
          
			if(CAPTURE_STA_TIM3CH[1]&0X40)		//捕获到一个下降沿 		
			{	  			
				CAPTURE_STA_TIM3CH[1] |= 0x80;                    //标记成功捕获到一次高电平脉宽
                CAPTURE_DOWN_TIM3CH[1] = TIM_GetCapture2(TIM3);
                if(CAPTURE_DOWN_TIM3CH[1] >= CAPTURE_UP_TIM3CH[1]) 
                    CAPTURE_VAL_TIM3CH[1] = CAPTURE_DOWN_TIM3CH[1] - CAPTURE_UP_TIM3CH[1];
                else 
                    CAPTURE_VAL_TIM3CH[1] = 0xffff + CAPTURE_DOWN_TIM3CH[1] - CAPTURE_UP_TIM3CH[1];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				CAPTURE_STA_TIM3CH[1]=0;			//清空
				CAPTURE_VAL_TIM3CH[1]=0;
	 			CAPTURE_UP_TIM3CH[1] = TIM_GetCapture2(TIM3);
				CAPTURE_STA_TIM3CH[1]|=0X40;		//标记捕获到了上升沿
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
    
    //通道三
 	if((CAPTURE_STA_TIM3CH[2]&0X80)==0)//还未成功捕获	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[2]&0X40)//已经捕获到高电平了
			{
				if((CAPTURE_STA_TIM3CH[2]&0X3F)==0X3F)//高电平太长了
				{
					CAPTURE_STA_TIM3CH[2]|=0X80;//标记成功捕获了一次
					CAPTURE_VAL_TIM3CH[2]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[2]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)//捕获1发生捕获事件
		{
          
			if(CAPTURE_STA_TIM3CH[2]&0X40)		//捕获到一个下降沿 		
			{	  			
				CAPTURE_STA_TIM3CH[2] |= 0x80;                    //标记成功捕获到一次高电平脉宽
                CAPTURE_DOWN_TIM3CH[2] = TIM_GetCapture3(TIM3);
                if(CAPTURE_DOWN_TIM3CH[2] >= CAPTURE_UP_TIM3CH[2]) 
                    CAPTURE_VAL_TIM3CH[2] = CAPTURE_DOWN_TIM3CH[2] - CAPTURE_UP_TIM3CH[2];
                else 
                    CAPTURE_VAL_TIM3CH[2] = 0xffff + CAPTURE_DOWN_TIM3CH[2] - CAPTURE_UP_TIM3CH[0];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				CAPTURE_STA_TIM3CH[2]=0;			//清空
				CAPTURE_VAL_TIM3CH[2]=0;
	 			CAPTURE_UP_TIM3CH[2] = TIM_GetCapture3(TIM3);
				CAPTURE_STA_TIM3CH[2]|=0X40;		//标记捕获到了上升沿
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
    
    //通道四
 	if((CAPTURE_STA_TIM3CH[3]&0X80)==0)//还未成功捕获	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[3]&0X40)//已经捕获到高电平了
			{
				if((CAPTURE_STA_TIM3CH[3]&0X3F)==0X3F)//高电平太长了
				{
					CAPTURE_STA_TIM3CH[3]|=0X80;//标记成功捕获了一次
					CAPTURE_VAL_TIM3CH[3]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[3]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)//捕获1发生捕获事件
		{
          
			if(CAPTURE_STA_TIM3CH[3]&0X40)		//捕获到一个下降沿 		
			{	  			
				CAPTURE_STA_TIM3CH[3] |= 0x80;                    //标记成功捕获到一次高电平脉宽
                CAPTURE_DOWN_TIM3CH[3] = TIM_GetCapture4(TIM3);
                if(CAPTURE_DOWN_TIM3CH[3] >= CAPTURE_UP_TIM3CH[3]) 
                    CAPTURE_VAL_TIM3CH[3] = CAPTURE_DOWN_TIM3CH[3] - CAPTURE_UP_TIM3CH[3];
                else 
                    CAPTURE_VAL_TIM3CH[3] = 0xffff + CAPTURE_DOWN_TIM3CH[3] - CAPTURE_UP_TIM3CH[3];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				CAPTURE_STA_TIM3CH[3]=0;			//清空
				CAPTURE_VAL_TIM3CH[3]=0;
	 			CAPTURE_UP_TIM3CH[3] = TIM_GetCapture4(TIM3);
				CAPTURE_STA_TIM3CH[3]|=0X40;		//标记捕获到了上升沿
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
 
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4|TIM_IT_Update); //清除中断标志位
 
}

void tim_print_result(void)
{
    uint16_t temp=0;
    float fre = 0.0;
    //static uin8_t frestopflag =0x00 ; //没有采集到数据的标志，
    #ifdef USEDEBUG
    //printf("in tim print test\r\n");
    #endif
 
    if(CAPTURE_STA_TIM3CH[0]&0X80)//成功捕获到了一次上升沿
		{
            temp = 0;
			temp=CAPTURE_STA_TIM3CH[0]&0X3F;
			temp*=65536;//溢出时间总和
			temp+=CAPTURE_VAL_TIM3CH[0];//得到总的高电平时间
			//bsp_Diwen_Updatedata(0x000A,10000/temp);  
            #ifdef USEDEBUG
            printf("通道1 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//打印总的高点平时间
            #endif
			CAPTURE_STA_TIM3CH[0]=0;//开启下一次捕获 
            fre = 1000.0/temp;
            if((fre>=CHECKFRELOW) &&(fre<=CHECKFREHIGH))  g_checkfreok[0]++;
          
		}
        else  //
        {
           // bsp_Diwen_Updatedata(0x000A,0);  
        }
        
     if(CAPTURE_STA_TIM3CH[1]&0X80)//成功捕获到了一次上升沿
		{
            temp = 0;
			temp=CAPTURE_STA_TIM3CH[1]&0X3F;
			temp*=65536;//溢出时间总和
			temp+=CAPTURE_VAL_TIM3CH[1];//得到总的高电平时间
			//bsp_Diwen_Updatedata(0x000B,10000/temp);  
            #ifdef USEDEBUG
            printf("通道2 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//打印总的高点平时间
            #endif
			CAPTURE_STA_TIM3CH[1]=0;//开启下一次捕获
             fre = 1000.0/temp;
            if((fre>=CHECKFRELOW) &&(fre<=CHECKFREHIGH))  g_checkfreok[1]++;
          
		} 
        else  //
        {
           // bsp_Diwen_Updatedata(0x000B,0);  
        }
        
      if(CAPTURE_STA_TIM3CH[2]&0X80)//成功捕获到了一次上升沿
		{
            temp = 0;
			temp=CAPTURE_STA_TIM3CH[2]&0X3F;
			temp*=65536;//溢出时间总和
			temp+=CAPTURE_VAL_TIM3CH[2];//得到总的高电平时间
			//bsp_Diwen_Updatedata(0x000C,10000/temp); 
            #ifdef USEDEBUG            
            printf("通道3 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//打印总的高点平时间
            #endif
			CAPTURE_STA_TIM3CH[2]=0;//开启下一次捕获
             fre = 1000.0/temp;
            if((fre>=CHECKFRELOW) &&(fre<=CHECKFREHIGH))  g_checkfreok[2]++;
          
		}
        else  //
        {
          //  bsp_Diwen_Updatedata(0x000C,0);  
        }
        
      if(CAPTURE_STA_TIM3CH[3]&0X80)//成功捕获到了一次上升沿
		{
            temp = 0;
			temp=CAPTURE_STA_TIM3CH[3]&0X3F;
			temp*=65536;//溢出时间总和
			temp+=CAPTURE_VAL_TIM3CH[3];//得到总的高电平时间
			//bsp_Diwen_Updatedata(0x000D,10000/temp);  
            #ifdef USEDEBUG
            printf("通道4 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//打印总的高点平时间
            #endif
			CAPTURE_STA_TIM3CH[3]=0;//开启下一次捕获
             fre = 1000.0/temp;
            if((fre>=CHECKFRELOW) &&(fre<=CHECKFREHIGH))  g_checkfreok[3]++;
          
		} 
        else  //
        {
           // bsp_Diwen_Updatedata(0x000D,0);  
        }
        
    
}

