
#include "bsp.h"

/* define ---------------------------------------------------------------------*/
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define ADC2_DR_Address    ((uint32_t)0x4001284C)
//#define ADC3_DR_Address    ((uint32_t)0x40013C4C)
#define  SAMPLE_COUNT  2*1024
uint16_t Ch1Buf[2048]={0};	 /* ͨ��1���ݻ����� */
uint16_t StoreBuf[3*1024] ={0};
static void SetSampRate(uint32_t freq) ;
uint32_t DmaTctime =0;
uint16_t ADCSampleTime=0;
uint16_t ADCSampleTimeSave =0;
uint8_t  Phaseindex =0;
 void bsp_InitADC(void)
{
	/* ����GPIO.  PA1*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		/* ��GPIO_A �� AFIO ��ʱ�� */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA	| RCC_APB2Periph_AFIO, ENABLE);
		/* ����PC0Ϊģ������ģʽ */
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	/* ����DMA1_1 ����CH1 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;
		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		DMA_DeInit(DMA1_Channel1);		/* ��λDMA1�Ĵ�����ȱʡ״̬ */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* ѡ��ADC1�����ݼĴ�����ΪԴ */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Ch1Buf;	/* Ŀ���ַ */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* ����DMA���䷽������(ADC)��ΪԴ */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* ���û�������С */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* �����ַ������ */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* �洢����ַ��Ҫ���� */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* ѡ�����贫�䵥λ��16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* ѡ���ڴ洫�䵥λ��16bit */
		//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* ����ѭ��ģʽ   */
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	/* ѭ��ģʽ   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* ѡ��DMA���ȼ� */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA�������ͣ������ڴ浽�ڴ� */
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);
        
        DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);  //ʹ��DMA��������ж�
		/* Enable DMA1 channel1 */
		DMA_Cmd(DMA1_Channel1, ENABLE);
	}
    /* ����DMA�ж�ʹ��   */
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
    
	/* ����ADC1  */

	 /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);     //ADCCLK = PCLK2/4 = 18MHz

	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* ����ת����ֹ */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* ѡ��TIM1��CC3������ */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	/* ѡ��TIM2��CC2������ */
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* �����Ҷ���,��λΪ0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1��ͨ�� */
		ADC_Init(ADC1, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC1 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC1, ENABLE);

		/* ʹ�� ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Enable ADC1 DMA */
		ADC_DMACmd(ADC1, ENABLE);

		/* ʹ�� ADC1 ��λУ׼�Ĵ��� */
		ADC_ResetCalibration(ADC1);
		/* ��⸴λУ׼�Ĵ��� */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* ��ʼ ADC1 У׼ */
		ADC_StartCalibration(ADC1);
		/* �ȴ�У׼���� */
		while(ADC_GetCalibrationStatus(ADC1));
	}
	#endif


	/* ���ò���������ʱ����ʹ��TIM1 CC1 */
	SetSampRate(10000);	/* �޸Ĳ���Ƶ��(����ʱ100K) */ //7200 0000
}

/*
*********************************************************************************************************
*	�� �� ��: SetSampRate
*	����˵��: �޸Ĳ���Ƶ��
*	��    ��: freq : ����Ƶ�� ��λHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SetSampRate(uint32_t freq)
{
	/* ���ö�ʱ��2��2ͨ��CC2, CC2��ΪADC1�Ĵ���Դ */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t Period;

	TIM_Cmd(TIM1, DISABLE);

//	g_DSO->SampleRate = freq;	/* ������Ƶ�ʱ��浽ȫ�������� */

	/*
	����Ƶ�ʼ��㹫ʽ ��
		period = 72000000 / freq ;
		
		1200 == 60KHz ����Ƶ��
	*/
	Period = 72000000 / freq;		

	/* Enable TIM1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = Period;          
	TIM_TimeBaseStructure.TIM_Prescaler = 0;   /* ����Ƶ�� = 72000 000 / 18 = 4000 000	 */
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
        DmaTctime++; //��ɵ�dma�жϴ���
        ADCSampleTime=0;//��¼��ǰ��¼��������� ����
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
           
        DMA_ClearITPendingBit(DMA1_IT_GL1); //���ȫ���жϱ�־
        
    }
}

void TIM1_CC_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1,TIM_IT_CC3))
    {
        ADCSampleTime++;//���������++ �����趨��Ƶ�ʽ��в�������һ�� ��һ��  ����Ƶ�ʣ�100k 10k 1k �� ��
//        if(ADCSampleTime%100 ==0)
//           bsp_LedToggle(3);
        TIM_ClearITPendingBit(TIM1,TIM_IT_CC3); //���ȫ���жϱ�־
    }
}

 
