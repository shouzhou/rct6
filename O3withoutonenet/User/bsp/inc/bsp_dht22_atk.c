#include "bsp.h"

//复位DHT22
void DHT22_Rst(void)	   
{                 
	DHT22_IO_OUT(); 	//SET OUTPUT
	DHT22_DQ_OUT=0; 	//拉低DQ
	bsp_DelayMS(18);    	//拉低至少500us
	DHT22_DQ_OUT=1; 	//DQ=1 
	bsp_DelayUS(30);     //主机拉高20~40us
}
//等待DHT22的回应
//返回1:未检测到DHT22的存在
//返回0:存在
uint8_t DHT22_Check(void) 	   
{   
	uint8_t retry=0;
	DHT22_IO_IN();//SET INPUT	 
  while (DHT22_DQ_IN&&retry<100)//DHT22会拉低80us左右
	{
		retry++;
		bsp_DelayUS(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
  while (!DHT22_DQ_IN&&retry<100)//DHT22拉高后会再次拉高80us左右
	{
		retry++;
		bsp_DelayUS(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}
//从DHT22读取一个位
//返回值：1/0
uint8_t DHT22_Read_Bit(void) 			 
{
 	uint8_t retry=0;
	while(DHT22_DQ_IN&&retry<100)//等待变为低电平
	{
		retry++;
		bsp_DelayUS(1);
	}
	retry=0;
	while(!DHT22_DQ_IN&&retry<100)//等待变高电平
	{
		retry++;
		bsp_DelayUS(1);
	}
	bsp_DelayUS(40);//等待40us
	if(DHT22_DQ_IN)return 1;
	else return 0;		   
}
//从DHT22读取一个字节
//返回值：读到的数据
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
//从DHT22读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
uint8_t DHT22_Read_Data(uint16_t *temp,uint16_t *humi)
{
 	uint8_t buf[5];
	uint8_t i;
	DHT22_Rst();
	if(DHT22_Check()==0)
	{
      //  printf("in\r\n");
		for(i=0;i<5;i++)//读取40位数据
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
//初始化DHT22的IO口 DQ 同时检测DHT22的存在
//返回1:不存在
//返回0:存在    	 
uint8_t DHT22_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB端口时钟
	
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				  //PB3端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);				    //初始化IO口
 	GPIO_SetBits(GPIOA,GPIO_Pin_12);						        //PB3 输出高
			    
	DHT22_Rst();  //复位DHT22
	return DHT22_Check();//等待DHT22的回应
} 
