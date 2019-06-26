#include "bsp.h"
uint16_t count=0;
// �����ҪTIM3��ӳ�� ���򿪸ú궨�弴��
#define  TIM3REMAP    
void  BASIC_TIM_IRQHandler (void)
{
	if ( TIM_GetITStatus( BASIC_TIM, TIM_IT_Update) != RESET ) 
	{	
        count++;
        // ������������ ��ͬ��Ƶ�� ��������Ƶ�ź�
     //   if(count%3==0)
        bsp_LedToggle(1);
		bsp_LedToggle(2);
      //  if(count%10==0)
        bsp_LedToggle(3);
      //  if(count%50==0)
        bsp_LedToggle(4);
		TIM_ClearITPendingBit(BASIC_TIM , TIM_FLAG_Update);  		 
	}		 	
}
// �ж����ȼ�����
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
///////////////////////TIM3ԭʼ����ͨ�� /////////////////////
//PA6 --TIM3CH1
//PA7 --TIM3CH2
//PB0 --TIM3CH3
//PB1 --TIM3CH4

///////////////////////TIM3��ӳ�����ͨ�� /////////////////////
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
    //ԭʼ����
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ��TIM3ʱ��
 
    #else 
  //��ӳ������  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3������ӳ��  TIM3_CH2->PB5  
    #endif
    
  
   #ifndef TIM3REMAP    
    // ԭʼ����
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 ����  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	#else
//  REMAP ���� 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //P60 ����  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   #endif

    
	//��ʼ����ʱ��5 TIM3	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM3���벶�����
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
    
    TIM3_ICInitStructure.TIM_Channel = TIM_Channel_4; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�2��
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
   	TIM_Cmd(TIM3,ENABLE ); 	//ʹ�ܶ�ʱ��5
   


}

//u8  CAPTURE_STA_TIM3CH[0]=0;	//���벶��״̬		    				
//u16	CAPTURE_VAL_TIM3CH[0];	//���벶��ֵ
u8 CAPTURE_STA_TIM3CH[4] = {0};
u16 CAPTURE_VAL_TIM3CH[4];
u16 CAPTURE_UP_TIM3CH[4], CAPTURE_DOWN_TIM3CH[4];
//��ʱ��5�жϷ������	 
void TIM3_IRQHandler(void)
{ 
   //ͨ��һ
 	if((CAPTURE_STA_TIM3CH[0]&0X80)==0)//��δ�ɹ�����	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[0]&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((CAPTURE_STA_TIM3CH[0]&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					CAPTURE_STA_TIM3CH[0]|=0X80;//��ǳɹ�������һ��
					CAPTURE_VAL_TIM3CH[0]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[0]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)//����1���������¼�
		{
          
			if(CAPTURE_STA_TIM3CH[0]&0X40)		//����һ���½��� 		
			{	  			
				CAPTURE_STA_TIM3CH[0] |= 0x80;                    //��ǳɹ�����һ�θߵ�ƽ����
                CAPTURE_DOWN_TIM3CH[0] = TIM_GetCapture1(TIM3);
                if(CAPTURE_DOWN_TIM3CH[0] >= CAPTURE_UP_TIM3CH[0]) 
                    CAPTURE_VAL_TIM3CH[0] = CAPTURE_DOWN_TIM3CH[0] - CAPTURE_UP_TIM3CH[0];
                else 
                    CAPTURE_VAL_TIM3CH[0] = 0xffff + CAPTURE_DOWN_TIM3CH[0] - CAPTURE_UP_TIM3CH[0];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				CAPTURE_STA_TIM3CH[0]=0;			//���
				CAPTURE_VAL_TIM3CH[0]=0;
	 			CAPTURE_UP_TIM3CH[0] = TIM_GetCapture1(TIM3);
				CAPTURE_STA_TIM3CH[0]|=0X40;		//��ǲ�����������
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
    //ͨ����  
   if((CAPTURE_STA_TIM3CH[1]&0X80)==0)//��δ�ɹ�����	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[1]&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((CAPTURE_STA_TIM3CH[1]&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					CAPTURE_STA_TIM3CH[1]|=0X80;//��ǳɹ�������һ��
					CAPTURE_VAL_TIM3CH[1]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[1]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)//����1���������¼�
		{
          
			if(CAPTURE_STA_TIM3CH[1]&0X40)		//����һ���½��� 		
			{	  			
				CAPTURE_STA_TIM3CH[1] |= 0x80;                    //��ǳɹ�����һ�θߵ�ƽ����
                CAPTURE_DOWN_TIM3CH[1] = TIM_GetCapture2(TIM3);
                if(CAPTURE_DOWN_TIM3CH[1] >= CAPTURE_UP_TIM3CH[1]) 
                    CAPTURE_VAL_TIM3CH[1] = CAPTURE_DOWN_TIM3CH[1] - CAPTURE_UP_TIM3CH[1];
                else 
                    CAPTURE_VAL_TIM3CH[1] = 0xffff + CAPTURE_DOWN_TIM3CH[1] - CAPTURE_UP_TIM3CH[1];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				CAPTURE_STA_TIM3CH[1]=0;			//���
				CAPTURE_VAL_TIM3CH[1]=0;
	 			CAPTURE_UP_TIM3CH[1] = TIM_GetCapture2(TIM3);
				CAPTURE_STA_TIM3CH[1]|=0X40;		//��ǲ�����������
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
    
    //ͨ����
 	if((CAPTURE_STA_TIM3CH[2]&0X80)==0)//��δ�ɹ�����	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[2]&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((CAPTURE_STA_TIM3CH[2]&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					CAPTURE_STA_TIM3CH[2]|=0X80;//��ǳɹ�������һ��
					CAPTURE_VAL_TIM3CH[2]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[2]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)//����1���������¼�
		{
          
			if(CAPTURE_STA_TIM3CH[2]&0X40)		//����һ���½��� 		
			{	  			
				CAPTURE_STA_TIM3CH[2] |= 0x80;                    //��ǳɹ�����һ�θߵ�ƽ����
                CAPTURE_DOWN_TIM3CH[2] = TIM_GetCapture3(TIM3);
                if(CAPTURE_DOWN_TIM3CH[2] >= CAPTURE_UP_TIM3CH[2]) 
                    CAPTURE_VAL_TIM3CH[2] = CAPTURE_DOWN_TIM3CH[2] - CAPTURE_UP_TIM3CH[2];
                else 
                    CAPTURE_VAL_TIM3CH[2] = 0xffff + CAPTURE_DOWN_TIM3CH[2] - CAPTURE_UP_TIM3CH[0];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				CAPTURE_STA_TIM3CH[2]=0;			//���
				CAPTURE_VAL_TIM3CH[2]=0;
	 			CAPTURE_UP_TIM3CH[2] = TIM_GetCapture3(TIM3);
				CAPTURE_STA_TIM3CH[2]|=0X40;		//��ǲ�����������
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
    
    //ͨ����
 	if((CAPTURE_STA_TIM3CH[3]&0X80)==0)//��δ�ɹ�����	
	{	 
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{	
        	if(CAPTURE_STA_TIM3CH[3]&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((CAPTURE_STA_TIM3CH[3]&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					CAPTURE_STA_TIM3CH[3]|=0X80;//��ǳɹ�������һ��
					CAPTURE_VAL_TIM3CH[3]=0XFFFF;
				}else CAPTURE_STA_TIM3CH[3]++;
			}	 
		}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)//����1���������¼�
		{
          
			if(CAPTURE_STA_TIM3CH[3]&0X40)		//����һ���½��� 		
			{	  			
				CAPTURE_STA_TIM3CH[3] |= 0x80;                    //��ǳɹ�����һ�θߵ�ƽ����
                CAPTURE_DOWN_TIM3CH[3] = TIM_GetCapture4(TIM3);
                if(CAPTURE_DOWN_TIM3CH[3] >= CAPTURE_UP_TIM3CH[3]) 
                    CAPTURE_VAL_TIM3CH[3] = CAPTURE_DOWN_TIM3CH[3] - CAPTURE_UP_TIM3CH[3];
                else 
                    CAPTURE_VAL_TIM3CH[3] = 0xffff + CAPTURE_DOWN_TIM3CH[3] - CAPTURE_UP_TIM3CH[3];
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				CAPTURE_STA_TIM3CH[3]=0;			//���
				CAPTURE_VAL_TIM3CH[3]=0;
	 			CAPTURE_UP_TIM3CH[3] = TIM_GetCapture4(TIM3);
				CAPTURE_STA_TIM3CH[3]|=0X40;		//��ǲ�����������
		   	//	TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
 
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4|TIM_IT_Update); //����жϱ�־λ
 
}

void tim_print_result(void)
{
    uint16_t temp=0;
  //  bsp_Diwen_Updatedata(0x0007,temp);
   // printf("interestin");
    if(CAPTURE_STA_TIM3CH[0]&0X80)//�ɹ�������һ��������
		{
			temp=CAPTURE_STA_TIM3CH[0]&0X3F;
			temp*=65536;//���ʱ���ܺ�
			temp+=CAPTURE_VAL_TIM3CH[0];//�õ��ܵĸߵ�ƽʱ��
			//bsp_Diwen_Updatedata(0x0007,temp);  
            printf("ͨ��1 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//��ӡ�ܵĸߵ�ƽʱ��
			CAPTURE_STA_TIM3CH[0]=0;//������һ�β���
          //  TIM_Cmd(TIM3,DISABLE);
		}
        
     if(CAPTURE_STA_TIM3CH[1]&0X80)//�ɹ�������һ��������
		{
			temp=CAPTURE_STA_TIM3CH[1]&0X3F;
			temp*=65536;//���ʱ���ܺ�
			temp+=CAPTURE_VAL_TIM3CH[1];//�õ��ܵĸߵ�ƽʱ��
			//bsp_Diwen_Updatedata(0x0007,temp);  
            printf("ͨ��2 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//��ӡ�ܵĸߵ�ƽʱ��
			CAPTURE_STA_TIM3CH[1]=0;//������һ�β���
          //  TIM_Cmd(TIM3,DISABLE);
		} 
        
      if(CAPTURE_STA_TIM3CH[2]&0X80)//�ɹ�������һ��������
		{
			temp=CAPTURE_STA_TIM3CH[2]&0X3F;
			temp*=65536;//���ʱ���ܺ�
			temp+=CAPTURE_VAL_TIM3CH[2];//�õ��ܵĸߵ�ƽʱ��
			//bsp_Diwen_Updatedata(0x0007,temp);  
            printf("ͨ��3 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//��ӡ�ܵĸߵ�ƽʱ��
			CAPTURE_STA_TIM3CH[2]=0;//������һ�β���
          //  TIM_Cmd(TIM3,DISABLE);
		} 
        
      if(CAPTURE_STA_TIM3CH[3]&0X80)//�ɹ�������һ��������
		{
			temp=CAPTURE_STA_TIM3CH[3]&0X3F;
			temp*=65536;//���ʱ���ܺ�
			temp+=CAPTURE_VAL_TIM3CH[3];//�õ��ܵĸߵ�ƽʱ��
			//bsp_Diwen_Updatedata(0x0007,temp);  
            printf("ͨ��4 :HIGH:%d us  FREQUENCE :%4.2fKHz\r\n",temp,1000.0/(float)temp);	//��ӡ�ܵĸߵ�ƽʱ��
			CAPTURE_STA_TIM3CH[3]=0;//������һ�β���
          //  TIM_Cmd(TIM3,DISABLE);
		} 
        
    
}

