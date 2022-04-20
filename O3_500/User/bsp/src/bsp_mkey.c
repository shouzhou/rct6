#include "bsp.h"
// 
//               |------------------|
//PB15--key8-----|                  |
//PB14--key7-----|                  |
//PB13--key6-----|                  |
//PB12--key5-----|                  |
//PC5 --key4-----|                  |
//PC4 --key3-----|                  |
//PA7 --key2-----|                  |
//PA6 --key1-----|                  |
//               |------------------|
uint8_t keyvalue=0;

typedef struct 
{
	GPIO_TypeDef* GPIOX;
	uint16_t GPIO_Pin;
}KGPT[2][4]; //4*4
KGPT KPIN={ //ֻ��Ҫ��������д�����е�IO�ں͹ܽ�   IO�ں͹ܽŶ��������ⶨ�� ������һ��IO�� �ܽ�Ҳ���谴˳��
						{{GPIOA,GPIO_Pin_6} ,{GPIOA,GPIO_Pin_7}, {GPIOC,GPIO_Pin_4},{GPIOC,GPIO_Pin_5}},//row ����
						{{GPIOB,GPIO_Pin_12},{GPIOB,GPIO_Pin_13}, {GPIOB,GPIO_Pin_14},{GPIOB,GPIO_Pin_15}}  //col ����
//                        {{GPIOB,GPIO_Pin_12},{GPIOB,GPIO_Pin_13}, {GPIOB,GPIO_Pin_14},{GPIOB,GPIO_Pin_15}} , //col ���� 
//                        {{GPIOA,GPIO_Pin_6} ,{GPIOA,GPIO_Pin_7}, {GPIOC,GPIO_Pin_4},{GPIOC,GPIO_Pin_5}}//row ����
						
    
					};
#define row 0
#define col 1					
#define RCC_APB2_GPIOX  RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC//IOʱ��					
//#define MAX_Time   36000000 // ���ּ�������ʱ   
//������ʼ������ 
//������ʼ������ 
void bsp_MkeyInit(void)
{
  uint8_t i;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2_GPIOX,ENABLE);//ʹ��PORTCʱ��
  for(i=0;i<4;i++)
	
	{
	 GPIO_InitStructure.GPIO_Pin = KPIN[row][i].GPIO_Pin;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //���ó��������
	 GPIO_Init(KPIN[row][i].GPIOX, &GPIO_InitStructure);//��ʼ��row
	 GPIO_ResetBits(KPIN[row][i].GPIOX,KPIN[row][i].GPIO_Pin);
	}
 for(i=0;i<4;i++)
	{
     GPIO_InitStructure.GPIO_Pin  = KPIN[col][i].GPIO_Pin;//PC5
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
     GPIO_Init(KPIN[col][i].GPIOX, &GPIO_InitStructure);//��ʼ��col
	}
    printf("mkey init ok\r\n");
} 

uint8_t ReScan(GPIO_TypeDef* GPIOX,uint16_t colPin,uint8_t colIndex)
{
 uint8_t i;
// delay_ms(10);//������ʱ
 for(i = 0;i<4;i++)
    {
    GPIO_SetBits(KPIN[row][i].GPIOX,KPIN[row][i].GPIO_Pin);//ÿ������1
    if((GPIO_ReadInputDataBit(GPIOX, colPin) == 1)) //�����Ҳ���� �е�ֵ��֪���� Ϊ i
		{
			GPIO_ResetBits(KPIN[row][i].GPIOX,KPIN[row][i].GPIO_Pin); //�лָ� ��0 
            return colIndex+i*4+1;//���ص����� Ϊ1-16 ��Ӧ4x4���̵�16����
		}
	GPIO_ResetBits(KPIN[row][i].GPIOX,KPIN[row][i].GPIO_Pin);     //�лָ� ��0
    }
 return 0;
}
uint8_t bsp_MkeyScan(void)
{	 
 uint8_t i,key;

    #ifdef USEDEBUG
  //  printf("DEBUG ---in MkeyScan \r\n");
    #endif
	for(i = 0;i<4;i++)
	{
	 if(GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0)//����� ��ֵΪ i
	 {
         
			key = ReScan(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin,i);//����� ȡ��ֵ
		  while(GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0);
		//	while((GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0)&&(j<MAX_Time))j++;//���ּ��
			return key;//���ؼ�ֵ
     }
	}
return 0;
}

void  bsp_KeyFunction(uint8_t key)
{
    uint8_t temp;
    temp = key;
//    if(CurMenuIndex==1)  //��һ������
//    {
      switch(temp)
      {
     
            case  KEYPAD_A : //�������
//                bsp_EnableTouch();
//                bsp_SendKey(0x02);
                g_touch = 6;
                printf("keyA press11\r\n");
                break;
            case  KEYPAD_D : //�����豸
                g_funcKey = STARTAUTOV;
                if(g_status<10)
                    g_status = STATUS_AUTOV_START;
                printf("keyD press11\r\n");
                break;
            case  KEYPAD_1 : //�޷���
                
                g_chemical = 0;

            
                printf("key1 press11\r\n");
                g_touch = 7;
                break;
            case  KEYPAD_2 : //�з���
               
                g_chemical = 1;
           
                printf("key2 press11\r\n");
                //bsp_ScreenUPdateChemical(g_chemical);
                g_touch = 8;
                break;
            case  KEYPAD_4 : //����
              
                g_plant = 0;
            
                         
                printf("key4 press11\r\n");
              //  bsp_ScreenUPdateType(g_plant);
               g_touch = 9;
                break;
            case  KEYPAD_5 : //������
                
                g_plant = 1;
            
                           
               // bsp_ScreenUPdateType(g_plant);
                printf("key5 press11\r\n");
                g_touch = 10;
                break;
            default :
                break;
      }  
  }


					

