#include "bsp.h"

//��λDHT22
void DHT22_Rst(void)	   
{                 
	DHT22_IO_OUT(); 	//SET OUTPUT
	DHT22_DQ_OUT=0; 	//����DQ
	bsp_DelayMS(18);    	//��������500us
	DHT22_DQ_OUT=1; 	//DQ=1 
	bsp_DelayUS(30);     //��������20~40us
}
//�ȴ�DHT22�Ļ�Ӧ
//����1:δ��⵽DHT22�Ĵ���
//����0:����
uint8_t DHT22_Check(void) 	   
{   
	uint8_t retry=0;
	DHT22_IO_IN();//SET INPUT	 
  while (DHT22_DQ_IN&&retry<100)//DHT22������80us����
	{
		retry++;
		bsp_DelayUS(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
  while (!DHT22_DQ_IN&&retry<100)//DHT22���ߺ���ٴ�����80us����
	{
		retry++;
		bsp_DelayUS(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}
//��DHT22��ȡһ��λ
//����ֵ��1/0
uint8_t DHT22_Read_Bit(void) 			 
{
 	uint8_t retry=0;
	while(DHT22_DQ_IN&&retry<100)//�ȴ���Ϊ�͵�ƽ
	{
		retry++;
		bsp_DelayUS(1);
	}
	retry=0;
	while(!DHT22_DQ_IN&&retry<100)//�ȴ���ߵ�ƽ
	{
		retry++;
		bsp_DelayUS(1);
	}
	bsp_DelayUS(40);//�ȴ�40us
	if(DHT22_DQ_IN)return 1;
	else return 0;		   
}
//��DHT22��ȡһ���ֽ�
//����ֵ������������
uint8_t DHT22_Read_Byte(void)    
{        
    uint8_t i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT22_Read_Bit();
	}						    
	return dat;
}
//��DHT22��ȡһ������
//temp:�¶�ֵ(��Χ:0~50��)
//humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��0,����;1,��ȡʧ��
uint8_t DHT22_Read_Data(uint16_t *temp,uint16_t *humi)
{
 	uint8_t buf[5];
	uint8_t i;
	DHT22_Rst();
	if(DHT22_Check()==0)
	{
      //  printf("in\r\n");
		for(i=0;i<5;i++)//��ȡ40λ����
		{
			buf[i]=DHT22_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=(uint16_t)buf[0]<<8|buf[1];
			*temp=(uint16_t)buf[2]<<8|buf[3];
         //   printf("in\r\n");
		}
	}else return 1;
	return 0;	    
}
//��ʼ��DHT22��IO�� DQ ͬʱ���DHT22�Ĵ���
//����1:������
//����0:����    	 
uint8_t DHT22_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PB�˿�ʱ��
	
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				  //PB3�˿�����
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);				    //��ʼ��IO��
 	GPIO_SetBits(GPIOA,GPIO_Pin_12);						        //PB3 �����
			    
	DHT22_Rst();  //��λDHT22
	return DHT22_Check();//�ȴ�DHT22�Ļ�Ӧ
} 
