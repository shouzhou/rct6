
/*
	应用说明：访问串行EEPROM前，请先调用一次 bsp_InitI2C()函数配置好I2C相关的GPIO.
*/

#include "bsp.h"

uint8_t eeResult[EE_SIZE];  //用于保存系统数据

/*
    EEPROM 分配地址  ：6个字节存放当前的温湿度
    00 --0X00      
    //  第0位 == 1 表示 长度设置ok
    //  第1位 == 1 表示 宽度设置OK
    //  第2位 == 1 表示 高度设置OK
    01 --lengthH
    02 --lengthL
    03 --widthH
    04 --widthL
    05 --heightH
    06 --heightL
    07 --failtime
    //上次自检完成时间
    
    08--year
    09--month
    0a--day
    0b--hour
    0c--min
    0d--sec
    
    //
    0e--
    
    
    

*/
 
 

/*
*********************************************************************************************************
*	函 数 名: ee_CheckOk
*	功能说明: 判断串行EERPOM是否正常
*	形    参:  无
*	返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************
*/
uint8_t ee_CheckOk(void)
{
	if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		i2c_Stop();
		return 0;
	}
}


/*

  读取eeprom中的数据，在上电时做这个事情
*/

void ee_ReadSaveData(void)
{
    ee_ReadBytes(eeResult,0,20);
    if(eeResult[0] &0x01)  //长度已经设置
    {
        g_lenth = ((uint16_t)eeResult[1]<<8)+eeResult[2];
        #ifdef USEDEBUG

          printf("Read lenth = %d\r\n",g_lenth);
        
        #endif 
        bsp_Diwen_Updatedata(0x0001,g_lenth);
        bsp_DelayMS(100);
        bsp_Diwen_Updatedata(0x0001,g_lenth);
        bsp_DelayMS(100);
        
    }
    if(eeResult[0] &0x02)  //宽度已经设置
    {
          g_width = ((uint16_t)eeResult[3]<<8)+eeResult[4];
            #ifdef USEDEBUG

            printf("Read width = %d\r\n",g_width);
            
            #endif 
            bsp_Diwen_Updatedata(0x0002,g_width);
            bsp_DelayMS(100);
            bsp_Diwen_Updatedata(0x0002,g_width);
            bsp_DelayMS(100);
    }
    if(eeResult[0] &0x04)  //高度已经设置
    {
        g_height = ((uint16_t)eeResult[5]<<8)+eeResult[6];
        #ifdef USEDEBUG

        printf("Read height = %d\r\n",g_height);
        
        #endif 
        bsp_Diwen_Updatedata(0x0003,g_height);
        bsp_DelayMS(100);
        bsp_Diwen_Updatedata(0x0003,g_height);
        bsp_DelayMS(100);

    }
    g_FailTime =  eeResult[7];
    #ifdef USEDEBUG
    printf("fail time = %d\r\n",g_FailTime);
    #endif 
    if(g_FailTime >50) 
    {
        g_WithoutOnenet = 1;
      
    }
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadBytes
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i;

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	i2c_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	#if EE_ADDR_A8 == 1
		i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));	/* 此处是写指令 */
	#else
		i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
	#endif

	/* 第3步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	if (EE_ADDR_BYTES == 1)
	{
		i2c_SendByte((uint8_t)_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}
	else
	{
		i2c_SendByte(_usAddress >> 8);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		i2c_SendByte(_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	i2c_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	#if EE_ADDR_A8 == 1
		i2c_SendByte(EE_DEV_ADDR | I2C_RD | ((_usAddress >> 7) & 0x0E));	/* 此处是写指令 */
	#else		
		i2c_SendByte(EE_DEV_ADDR | I2C_RD);	/* 此处是写指令 */
	#endif	

	/* 第8步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第9步：循环读取数据 */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte();	/* 读1个字节 */

		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _usSize - 1)
		{
			i2c_Ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			i2c_NAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: ee_WriteBytes
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pWriteBuf : 存放读到的数据的缓冲区指针
			 _rease : 0 写_pWriteBuf数据， 1 写0xFF(0x00)
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_WriteBytes(uint8_t _rease, uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i,m;
	uint16_t usAddr;

	/*
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	/* 关闭写保护 */
	//WP = 0;

	usAddr = _usAddress;
	for (i = 0; i < _usSize; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			i2c_Stop();

			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
				CLK频率为200KHz时，查询次数为30次左右
			*/
			for (m = 0; m < 1000; m++)
			{
				/* 第1步：发起I2C总线启动信号 */
				i2c_Start();

				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				
				#if EE_ADDR_A8 == 1					/* 此处原版_______有BUG _usAddress */
					i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((usAddr >> 7) & 0x0E));	/* 此处是写指令 */
				#else				
					i2c_SendByte(EE_DEV_ADDR | I2C_WR);
				#endif

				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (i2c_WaitAck() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM器件写超时 */
			}

			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			if (EE_ADDR_BYTES == 1)
			{
				i2c_SendByte((uint8_t)usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
			else
			{
				i2c_SendByte(usAddr >> 8);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}

				i2c_SendByte(usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
		}

		/* 第6步：开始写入数据 */
		/* _rease>0?0xFF:_pWriteBuf[i]  夏欢欢修改 */
		//i2c_SendByte(_rease>0?0xFF:_pWriteBuf[i]);
        i2c_SendByte(_rease>0?0x00:_pWriteBuf[i]);
		/* 第7步：发送ACK */
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		usAddr++;	/* 地址增1 */
	}

	/* 命令执行成功，发送I2C总线停止信号 */
	i2c_Stop();

	/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
		CLK频率为200KHz时，查询次数为30次左右
	*/
	for (m = 0; m < 1000; m++)
	{
		/* 第1步：发起I2C总线启动信号 */
		i2c_Start();

		/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */	
		#if EE_ADDR_A8 == 1											/* 这个地方改不改有待调试 */
			i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));	/* 此处是写指令 */
		#else		
			i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
		#endif

		/* 第3步：发送一个时钟，判断器件是否正确应答 */
		if (i2c_WaitAck() == 0)
		{
			break;
		}
	}
	if (m  == 1000)
	{
		goto cmd_fail;	/* EEPROM器件写超时 */
	}

	/* 打开写保护 */
	//WP = 1;

	return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();

	/* 打开写保护 */
	//WP = 1;

	return 0;
}


