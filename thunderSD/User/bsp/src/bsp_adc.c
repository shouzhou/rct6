
#include "bsp.h"

/* define ---------------------------------------------------------------------*/
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define ADC2_DR_Address    ((uint32_t)0x4001284C)
//#define ADC3_DR_Address    ((uint32_t)0x40013C4C)
#define  SAMPLE_COUNT  2*1024
uint16_t Ch1Buf[2048]={0};	 /* 通道1数据缓冲区 */
uint16_t StoreBuf[3*1024] ={0};
static void SetSampRate(uint32_t freq) ;
uint32_t DmaTctime =0;
uint16_t ADCSampleTime=0;
uint16_t ADCSampleTimeSave =0;
uint8_t  Phaseindex =0;
 void bsp_InitADC(void)
{
	/* 配置GPIO.  PA1*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		/* 打开GPIO_A 和 AFIO 的时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA	| RCC_APB2Periph_AFIO, ENABLE);
		/* 配置PC0为模拟输入模式 */
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	/* 配置DMA1_1 用于CH1 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;
		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		DMA_DeInit(DMA1_Channel1);		/* 复位DMA1寄存器到缺省状态 */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* 选择ADC1的数据寄存器作为源 */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Ch1Buf;	/* 目标地址 */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* 设置DMA传输方向，外设(ADC)作为源 */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* 设置缓冲区大小 */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* 外设地址不自增 */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* 存储器地址需要自增 */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* 选择外设传输单位：16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* 选择内存传输单位：16bit */
		//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* 无需循环模式   */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	/* 循环模式   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* 选择DMA优先级 */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA传输类型，不是内存到内存 */
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);
        
        DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);  //使能DMA传输完成中断
		/* Enable DMA1 channel1 */
		DMA_Cmd(DMA1_Channel1, ENABLE);
	}
    /* 配置DMA中断使能   */
	#endif
    #if 1
    {
          NVIC_InitTypeDef NVIC_InitStructure;     /* Configure one bit for preemption priority */
        //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);     
          NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;     
          NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
          NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
          NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          
          NVIC_Init(&NVIC_InitStructure);
        
    }
    #endif
    
	/* 配置ADC1  */

	 /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);     //ADCCLK = PCLK2/4 = 18MHz

	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* 连续转换静止 */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* 选择TIM1的CC3做触发 */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	/* 选择TIM2的CC2做触发 */
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* 数据右对齐,高位为0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1个通道 */
		ADC_Init(ADC1, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC1 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC1, ENABLE);

		/* 使能 ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Enable ADC1 DMA */
		ADC_DMACmd(ADC1, ENABLE);

		/* 使能 ADC1 复位校准寄存器 */
		ADC_ResetCalibration(ADC1);
		/* 检测复位校准寄存器 */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* 开始 ADC1 校准 */
		ADC_StartCalibration(ADC1);
		/* 等待校准结束 */
		while(ADC_GetCalibrationStatus(ADC1));
	}
	#endif


	/* 配置采样触发定时器，使用TIM1 CC1 */
	SetSampRate(10000);	/* 修改采样频率(启动时100K) */ //7200 0000
}

/*
*********************************************************************************************************
*	函 数 名: SetSampRate
*	功能说明: 修改采样频率
*	形    参: freq : 采样频率 单位Hz
*	返 回 值: 无
*********************************************************************************************************
*/
static void SetSampRate(uint32_t freq)
{
	/* 配置定时器2第2通道CC2, CC2作为ADC1的触发源 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t Period;

	TIM_Cmd(TIM1, DISABLE);

//	g_DSO->SampleRate = freq;	/* 将采样频率保存到全部变量中 */

	/*
	采样频率计算公式 ：
		period = 72000000 / freq ;
		
		1200 == 60KHz 采样频率
	*/
	Period = 72000000 / freq;		

	/* Enable TIM1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = Period;          
	TIM_TimeBaseStructure.TIM_Prescaler = 0;   /* 计数频率 = 72000 000 / 18 = 4000 000	 */
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* TIM1 channel1 configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
	TIM_OCInitStructure.TIM_Pulse = TIM_TimeBaseStructure.TIM_Period / 2; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;         
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    
    TIM_ITConfig(TIM1,TIM_IT_CC3,ENABLE);
    
    #if 1
          NVIC_InitTypeDef NVIC_InitStructure;     /* Configure one bit for preemption priority */
        //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);     
          NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;     
          NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
          NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
          NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          
          NVIC_Init(&NVIC_InitStructure);
    #endif
    

	/* TIM1 counter enable */
	TIM_Cmd(TIM1, ENABLE);

	/* TIM1 main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  
}
 void StopADC(void)
{
	TIM_Cmd(TIM1, DISABLE);

	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, DISABLE);
	
	/* Enable DMA2 channel5 */
	//DMA_Cmd(DMA2_Channel5, DISABLE);

}

void StartADC(void)
{
  //  DmaTctime =0;
    ADCSampleTime=0;
    ADCSampleTimeSave =0;
    Phaseindex =0;
//    
//   	TIM_Cmd(TIM1, ENABLE);

//	/* Enable DMA1 channel1 */
//	DMA_Cmd(DMA1_Channel1, ENABLE); 
    bsp_InitADC();	

}

void DMA1_Channel1_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC1))
    {
        DmaTctime++; //完成的dma中断次数
        ADCSampleTime=0;//记录当前记录采样点变量 清零
        if(Phaseindex ==1)
        {
            Phaseindex =2;
            SaveData(Phaseindex,ADCSampleTimeSave);
        }
        else if(Phaseindex ==2)
        {
            Phaseindex =3; 
            SaveData(Phaseindex ,ADCSampleTimeSave);
           
        }
           
        DMA_ClearITPendingBit(DMA1_IT_GL1); //清除全部中断标志
        
    }
}

void TIM1_CC_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1,TIM_IT_CC3))
    {
        ADCSampleTime++;//采样点变量++ 根据设定的频率进行采样，采一次 加一，  采样频率：100k 10k 1k 等 等
//        if(ADCSampleTime%100 ==0)
//           bsp_LedToggle(3);
        TIM_ClearITPendingBit(TIM1,TIM_IT_CC3); //清除全部中断标志
    }
}

 
