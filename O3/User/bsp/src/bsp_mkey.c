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
#define  KEYPAD_1  16
#define  KEYPAD_2  12
#define  KEYPAD_3  8
#define  KEYPAD_4  15
#define  KEYPAD_5  11
#define  KEYPAD_6  7
#define  KEYPAD_7  14
#define  KEYPAD_8  10
#define  KEYPAD_9  6
#define  KEYPAD_0  9
#define  KEYPAD_A  4
#define  KEYPAD_B  3
#define  KEYPAD_C  2
#define  KEYPAD_D  1
#define  KEYPAD_STAR  13
#define  KEYPAD_HASH  5
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
#define MAX_Time   36000000 // ���ּ�������ʱ   
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
} 

uint8_t ReScan(GPIO_TypeDef* GPIOX,uint16_t colPin,uint8_t colIndex)
{
 uint8_t i;
// delay_ms(10);//������ʱ
 for(i = 0;i<4;i++){
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
 uint8_t i,keyvalue;
	//u32 j=0;
	for(i = 0;i<4;i++)
	{
	 if(GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0)//����� ��ֵΪ i
	 {
			keyvalue = ReScan(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin,i);//����� ȡ��ֵ
		  while(GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0);
		//	while((GPIO_ReadInputDataBit(KPIN[col][i].GPIOX,KPIN[col][i].GPIO_Pin) == 0)&&(j<MAX_Time))j++;//���ּ��
			return keyvalue;//���ؼ�ֵ
		}
	}
return 0;
}


					

