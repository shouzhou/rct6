 /********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <stdio.h>
#include "PLC_Dialogue.h"
#include "PLC_IO.h"
#include "PLC_CONF.H"

#define MB_Address     0x01   //从站地址 测试用

#define ReadY_Register             0x01 
#define ReadX_Register             0x02 
#define Read_Storage_Register      0x03 
#define Read_INput_Register        0x04
#define WriteForceY_statu          0x05  //强制单线圈
#define WriteForceY_status         0X0F  //强制多线圈
#define Write_Storage_Register     0x06  //预置一个寄存器
#define Write_Storage_Registers    0x10  //预置多个寄存器


volatile bool receive_flag;
u8  receive_flag1 = 0;
u8 Rx_header;
unsigned char TxBuffer3[100];
unsigned char RxBuffer3[100];
// short int d[6125]; //D寄存器
unsigned int crc;
unsigned char TxCounter3 = 0;
unsigned char RxCounter3 = 0;
unsigned char Send3_Size;
unsigned char ST_Scan3 = 0, ST_Scan3V = 0;
unsigned char CMUT, u485;
vu8 vk3 = 0;

// u8 RxBuffer3[201] =
// {
// 	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
// };

/* CRC 高位字节值表  const_执行时不能修改的变量值，code存储器空间*/
const unsigned char  auchCRCHi[10 *25+6] =
{
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
	0xC1, 0x81, 0x40
};
/* CRC低位字节值表*/
const unsigned char  auchCRCLo[10 *25+6] =
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05,
	0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A,
	0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B,
	0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14,
	0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11,
	0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36,
	0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF,
	0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28,
	0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D,
	0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22,
	0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63,
	0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C,
	0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69,
	0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE,
	0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77,
	0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50,
	0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55,
	0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A,
	0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B,
	0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44,
	0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41,
	0x81, 0x80, 0x40
};




unsigned int crc16(unsigned char *puchMsg, unsigned int usDataLen)
{
	unsigned char uchCRCHi = 0xFF; /* 高CRC字节初始化 */
	unsigned char uchCRCLo = 0xFF; /* 低CRC 字节初始化 */
	unsigned long uIndex; /* CRC循环中的索引 */
	while (usDataLen--)
	 /* 传输消息缓冲区 */
	{
		uIndex = uchCRCHi ^  *puchMsg++; /* 计算CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
} //uint16 crc16(uint8 *puchMsg, uint16 usDataLen)

void usart3(u16 DEFAULT_BAUD)
{
	USART_InitTypeDef USART_InitStructure;                          //串口配置寄存器	
	USART_InitStructure.USART_BaudRate = DEFAULT_BAUD;              //设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          //一位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;             //校验位 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART3,&USART_InitStructure);                        //初始化串口   
}  

//---------------------串口功能配置---------------------
void USART3_Configuration(void) 
{

	GPIO_InitTypeDef GPIO_InitStructure;                          //串口引脚配置寄存器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);       //打开串口对应的外设时钟  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
	
	   /* Configure USART3 Rx (PB11) as input floating                             */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure USART3 Tx (PB10) as alternate function push-pull                */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	usart3(19200);                                //初始化参数 
	//TXE发送中断,TC传输完成中断,RXNE接收中断,PE奇偶错误中断,可以是多个   
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);  
// 	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);//采用DMA方式发送
	USART_Cmd(USART3, ENABLE);                  //启动串口    
// 	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

  D8130 = 0;   //默认为从站
	D8121 = 1;   //默认从站地址为1

}

void modbus_back()   //485 MODBUS  RTU 20160502
{
	unsigned char P2, i;
	unsigned char svd[30];
	unsigned int k; //kp;	
	
  switch(RxBuffer3[1])
	{

		case ReadY_Register:	       //01 读取线圈状态 YY
		{
    svd[0] = RxBuffer3[0]; //ADDR
		svd[1] = RxBuffer3[1]; //FUN
		svd[2] = RxBuffer3[2];
		svd[3] = RxBuffer3[3];
//  		m = 1; //读取一组开关量字
		k = RxBuffer3[2];
		k = k * 256+RxBuffer3[3];  //读取起始地址

// 	 for (i = 0; i < m; i++)
// 		{
// 			svd[m+i]   = PLC_16BIT[80 +k]/256;
// 			svd[m+i+1] = PLC_16BIT[80 +k];
//     }

    svd[4] = PLC_16BIT[80 +k]/256;
		svd[5] = PLC_16BIT[80 +k];
			
		crc = crc16(svd, 6);
		svd[6] = crc / 256;
		svd[7] = crc;
		TxCounter3 = 0;
		Send3_Size = 0;
		for (i = 0; i <= 7; i++)
		TxBuffer3[Send3_Size++] = svd[i];
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		//RE1;u485=5;
		for (i = 0; i <= 10; i++)
			RxBuffer3[i] = 0;
		vk3 = 0;
		}
		break;
		
		case ReadX_Register:	       //02 读取开关量输入状态 XX 一组
		{
			svd[0] = RxBuffer3[0]; //ADDR
			svd[1] = RxBuffer3[1]; //FUN
			svd[2] = RxBuffer3[2];
			svd[3] = RxBuffer3[3];
				k = RxBuffer3[2];
				k = k * 256+RxBuffer3[3];  //读取起始地址

// 			 for (i = 0; i < svd[2]; i++)
// 				{
// 					svd[svd[2]+i]   = PLC_16BIT[64 +k]/256;
// 					svd[svd[2]+i+1] = PLC_16BIT[64 +k];
// 				}
			
        svd[4]   = PLC_16BIT[64 +k]/256;
 			  svd[5] = PLC_16BIT[64 +k];

				crc = crc16(svd, 6);
				svd[6] = crc / 256;
				svd[7] = crc;
				TxCounter3 = 0;
				Send3_Size = 0;
				for (i = 0; i <= 7; i++)
					TxBuffer3[Send3_Size++] = svd[i];
				USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
				//RE1;u485=5;
				for (i = 0; i <= 10; i++)
					RxBuffer3[i] = 0;
				vk3 = 0;
		}
		break;
		
  case Read_Storage_Register :    //03  读取保持寄存器
	case Read_INput_Register :      //04  读取输入寄存器
	{

		svd[0] = RxBuffer3[0]; //ADDR
		svd[1] = RxBuffer3[1]; //FUN
		svd[2] = RxBuffer3[5] *2; //BYTES
		k = RxBuffer3[2];
		k = k * 256+RxBuffer3[3];

		for (i = 0; i < RxBuffer3[5]; i++)
		{
			if ((k + i) <= 5999)
			{
				svd[3+i * 2] = PLC_16BIT[0x1000+i] / 256;
				svd[4+i * 2] = PLC_16BIT[0x1000+i];
			}

			if ((k + i) >= 8000)
			{
				svd[3+i * 2] = PLC_16BIT[k + i - 2000] / 256;
				svd[4+i * 2] = PLC_16BIT[k + i - 2000];
			}
		}

		crc = crc16(svd, svd[2] + 3);
		svd[svd[2] + 3] = crc / 256;
		svd[svd[2] + 4] = crc;
		TxCounter3 = 0;
		Send3_Size = 0;
		for (i = 0; i <= (svd[2] + 4); i++)
			TxBuffer3[Send3_Size++] = svd[i];
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		//RE1;u485=5;
		for (i = 0; i <= 10; i++)
			RxBuffer3[i] = 0;
		vk3 = 0;
	}
	break ;
		
		case Write_Storage_Register :   //06 预置一个寄存器
		{
			k = RxBuffer3[2];
			k = k * 256+RxBuffer3[3];
			if (k <= 5999)
			{
				PLC_16BIT[0x1000+k] = RxBuffer3[4];
				PLC_16BIT[0x1000+k] = PLC_16BIT[0x1000+k] *256;
				PLC_16BIT[0x1000+k] += RxBuffer3[5];
			}
			if (k >= 8000)
			{
	// 			k = k - 2000;
				PLC_16BIT[0x1000+k] = RxBuffer3[4];
				PLC_16BIT[0x1000+k] = PLC_16BIT[0x1000+k] *256;
				PLC_16BIT[0x1000+k] += RxBuffer3[5];
			}
			
			TxCounter3 = 0;
			Send3_Size = 0;
			for (i = 0; i <= 7; i++)
				TxBuffer3[Send3_Size++] = RxBuffer3[i];
			//设置单寄存器成功，数据原样返回。
			USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
			// RE1;u485=5;
			for (i = 0; i <= 10; i++)
				RxBuffer3[i] = 0;
			vk3 = 0;
		}
		break ;

	
		case WriteForceY_statu:	       //05 强制单线圈
		{
      k = RxBuffer3[2];
		  k = k * 256+RxBuffer3[3];
			PLC_16BIT[80 +k] = RxBuffer3[4]*256+RxBuffer3[5];
			
			TxCounter3 = 0;
			Send3_Size = 0;
			for (i = 0; i <= 7; i++)
				TxBuffer3[Send3_Size++] = RxBuffer3[i];
			//设置单寄存器成功，数据原样返回。
			USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
			// RE1;u485=5;
			for (i = 0; i <= 10; i++)
				RxBuffer3[i] = 0;
			vk3 = 0;
		}
		break;
		
		case WriteForceY_status:	       //0F 强制多线圈
		{
         
		}
		break;
		
	
	case Write_Storage_Registers :    //0X10  设置多个寄存器
	{
		k = RxBuffer3[2];
		k = k * 256+RxBuffer3[3];
		if (k <= 5999)
		{
			PLC_16BIT[0x1000+k] = RxBuffer3[7];
			PLC_16BIT[0x1000+k]= PLC_16BIT[0x1000+k] *256;
			PLC_16BIT[0x1000+k] += RxBuffer3[8];
		}
		if (k >= 8000)
		{
			PLC_16BIT[0x1000+k] = RxBuffer3[7];
			PLC_16BIT[0x1000+k]= PLC_16BIT[k - 2000] *256;
			PLC_16BIT[0x1000+k] += RxBuffer3[8];
		}

		TxCounter3 = 0;
		Send3_Size = 0;
		for (i = 0; i < 6; i++)
			TxBuffer3[Send3_Size++] = RxBuffer3[i];
		crc = crc16(RxBuffer3, 6);
		P2 = crc / 256;
		TxBuffer3[Send3_Size++] = P2;
		P2 = crc;
		TxBuffer3[Send3_Size++] = P2;
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		//RE1;u485=5;
		for (i = 0; i <= 10; i++)
			RxBuffer3[i] = 0;
		vk3 = 0;
	}
	break ;
	default :break ;
}
}

void mosbus_485_USART3(void)
{
	 u8 j;

		
   if(!receive_flag) //if receive over ,check crc
	 {
		if(receive_flag1 == 1)
		{
		  receive_flag1 = 0;
		if (D8130 == 0)  //从站 测试屏蔽
		{					
			if (RxBuffer3[0] == D8121)  //D8121是modbus 站地址	
			{
				
				if(RxCounter3 > 8) //11BIT
				{
					crc = crc16(RxBuffer3, 9);
					if ((crc % 256) == RxBuffer3[10]) //P=crc;
					if ((crc / 256) == RxBuffer3[9])  //Q=crc/256;
					//11 bit
					{
					   modbus_back();
// 						USART_SendData(USART3, 0X76);
					}
			  } 
				else
				{
					crc = crc16(RxBuffer3, 6); 
					if ((crc % 256) == RxBuffer3[7])  //P=crc;
					if ((crc / 256) == RxBuffer3[6])  //Q=crc/256;
					//8 bit
					{
	 				   modbus_back();
// 						USART_SendData(USART3, 0X56);
					}
        } 
			}
// 		}

		if (D8130 == 1)  //主站	
			{
				if (RxBuffer3[0] == D8122) //子站返回寄存器
				
				{
					crc = crc16(RxBuffer3, 5);

					if ((crc % 256) == RxBuffer3[6])
						if ((crc / 256) == RxBuffer3[5])
					//8 bit
					{
						PLC_16BIT[0x1000+78] = RxBuffer3[3] *256+RxBuffer3[4];
						for (j = 0; j <= 10; j++)
								RxBuffer3[j] = 0;
						 RxCounter3 = 0;
					}
				}
			}
   }
 }
	 		RxCounter3 = 0;
		 receive_flag =TRUE;
 }
}
/*******************************************************************************
 * Function Name  : USART3_IRQHandler
 * Description    : This function handles USART3 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USART3_IRQHandler(void)
{
	u8 sdata_sdata;

	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)//中断产生 
	{ 
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);//清除中断标志 
		if(receive_flag)
		{
			sdata_sdata=(u8)USART_ReceiveData(USART3); 
			if (!Rx_header)
			{
				RxBuffer3[0]=sdata_sdata; 
				Rx_header=TRUE;
			}
			if(RxBuffer3[0]==D8121) // 站号
			{
				RxBuffer3[RxCounter3]=(u8)USART_ReceiveData(USART3); 
				RxCounter3++;
				if(RxCounter3 >= 7)
				{
            receive_flag1 = 1;
        }
			}
		}
		
// 		RxBuffer3[RxCounter3++]=USART_ReceiveData(USART3);
// 		USART_SendData(USART3,USART_ReceiveData(USART3));
// 		while(USART_GetITStatus(USART3,USART_IT_RXNE)==RESET); //等待接收结束
	}

// 	 while(USART_GetFlagStatus(USART3, USART_FLAG_TXE)==RESET)
//     {
//     }

	
 
	 
	if(USART_GetFlagStatus(USART3,USART_FLAG_ORE)==SET)//溢出
	{
		USART_ClearFlag(USART3,USART_FLAG_ORE);//读SR
		USART_ReceiveData(USART3);//读DR
	}
	
	if(USART_GetITStatus(USART3, USART_IT_TXE)==SET)
	{
		USART_SendData(USART3, TxBuffer3[TxCounter3++]);
	 	if (TxCounter3 == Send3_Size)
		{
			/* Disable the USART3 Transmit interrupt */
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
// 			vk3=0;
// 			u485 = 10;
		}
		USART_ClearITPendingBit(USART3,USART_IT_TXE);
	}
	if(USART_GetITStatus(USART3, USART_IT_TC)==SET)  //发送中断
	{
	  USART_ClearITPendingBit(USART3,USART_IT_TC);
	}
	
}

