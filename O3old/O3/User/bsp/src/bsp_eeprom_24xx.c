
/*
	Ӧ��˵�������ʴ���EEPROMǰ�����ȵ���һ�� bsp_InitI2C()�������ú�I2C��ص�GPIO.
*/

#include "bsp.h"

uint8_t eeResult[EE_SIZE];  //���ڱ���ϵͳ����

/*
    EEPROM �����ַ  ��6���ֽڴ�ŵ�ǰ����ʪ��
    00 --0X00      
    //  ��0λ == 1 ��ʾ ��������ok
    //  ��1λ == 1 ��ʾ �������OK
    //  ��2λ == 1 ��ʾ �߶�����OK
    01 --lengthH
    02 --lengthL
    03 --widthH
    04 --widthL
    05 --heightH
    06 --heightL
    07 --failtime
    //�ϴ��Լ����ʱ��
    
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
*	�� �� ��: ee_CheckOk
*	����˵��: �жϴ���EERPOM�Ƿ�����
*	��    ��:  ��
*	�� �� ֵ: 1 ��ʾ������ 0 ��ʾ������
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
		/* ʧ�ܺ��мǷ���I2C����ֹͣ�ź� */
		i2c_Stop();
		return 0;
	}
}


/*

  ��ȡeeprom�е����ݣ����ϵ�ʱ���������
*/

void ee_ReadSaveData(void)
{
    ee_ReadBytes(eeResult,0,20);
    if(eeResult[0] &0x01)  //�����Ѿ�����
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
    if(eeResult[0] &0x02)  //����Ѿ�����
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
    if(eeResult[0] &0x04)  //�߶��Ѿ�����
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
*	�� �� ��: ee_ReadBytes
*	����˵��: �Ӵ���EEPROMָ����ַ����ʼ��ȡ��������
*	��    ��:  _usAddress : ��ʼ��ַ
*			 _usSize : ���ݳ��ȣ���λΪ�ֽ�
*			 _pReadBuf : ��Ŷ��������ݵĻ�����ָ��
*	�� �� ֵ: 0 ��ʾʧ�ܣ�1��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i;

	/* ���ô���EEPROM�漴��ȡָ�����У�������ȡ�����ֽ� */

	/* ��1��������I2C���������ź� */
	i2c_Start();

	/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	#if EE_ADDR_A8 == 1
		i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));	/* �˴���дָ�� */
	#else
		i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* �˴���дָ�� */
	#endif

	/* ��3��������ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM������Ӧ�� */
	}

	/* ��4���������ֽڵ�ַ��24C02ֻ��256�ֽڣ����1���ֽھ͹��ˣ������24C04���ϣ���ô�˴���Ҫ���������ַ */
	if (EE_ADDR_BYTES == 1)
	{
		i2c_SendByte((uint8_t)_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM������Ӧ�� */
		}
	}
	else
	{
		i2c_SendByte(_usAddress >> 8);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM������Ӧ�� */
		}

		i2c_SendByte(_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM������Ӧ�� */
		}
	}

	/* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
	i2c_Start();

	/* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	#if EE_ADDR_A8 == 1
		i2c_SendByte(EE_DEV_ADDR | I2C_RD | ((_usAddress >> 7) & 0x0E));	/* �˴���дָ�� */
	#else		
		i2c_SendByte(EE_DEV_ADDR | I2C_RD);	/* �˴���дָ�� */
	#endif	

	/* ��8��������ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM������Ӧ�� */
	}

	/* ��9����ѭ����ȡ���� */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte();	/* ��1���ֽ� */

		/* ÿ����1���ֽں���Ҫ����Ack�� ���һ���ֽڲ���ҪAck����Nack */
		if (i != _usSize - 1)
		{
			i2c_Ack();	/* �м��ֽڶ����CPU����ACK�ź�(����SDA = 0) */
		}
		else
		{
			i2c_NAck();	/* ���1���ֽڶ����CPU����NACK�ź�(����SDA = 1) */
		}
	}
	/* ����I2C����ֹͣ�ź� */
	i2c_Stop();
	return 1;	/* ִ�гɹ� */

cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	i2c_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: ee_WriteBytes
*	����˵��: ����EEPROMָ����ַд���������ݣ�����ҳд�������д��Ч��
*	��    ��:  _usAddress : ��ʼ��ַ
*			 _usSize : ���ݳ��ȣ���λΪ�ֽ�
*			 _pWriteBuf : ��Ŷ��������ݵĻ�����ָ��
			 _rease : 0 д_pWriteBuf���ݣ� 1 д0xFF(0x00)
*	�� �� ֵ: 0 ��ʾʧ�ܣ�1��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t ee_WriteBytes(uint8_t _rease, uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i,m;
	uint16_t usAddr;

	/*
		д����EEPROM�������������������ȡ�ܶ��ֽڣ�ÿ��д����ֻ����ͬһ��page��
		����24xx02��page size = 8
		�򵥵Ĵ�����Ϊ�����ֽ�д����ģʽ��ÿд1���ֽڣ������͵�ַ
		Ϊ���������д��Ч��: ����������page wirte������
	*/

	/* �ر�д���� */
	//WP = 0;

	usAddr = _usAddress;
	for (i = 0; i < _usSize; i++)
	{
		/* �����͵�1���ֽڻ���ҳ���׵�ַʱ����Ҫ���·��������źź͵�ַ */
		if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
		{
			/*���ڣ�������ֹͣ�źţ������ڲ�д������*/
			i2c_Stop();

			/* ͨ���������Ӧ��ķ�ʽ���ж��ڲ�д�����Ƿ����, һ��С�� 10ms
				CLKƵ��Ϊ200KHzʱ����ѯ����Ϊ30������
			*/
			for (m = 0; m < 1000; m++)
			{
				/* ��1��������I2C���������ź� */
				i2c_Start();

				/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
				
				#if EE_ADDR_A8 == 1					/* �˴�ԭ��_______��BUG _usAddress */
					i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((usAddr >> 7) & 0x0E));	/* �˴���дָ�� */
				#else				
					i2c_SendByte(EE_DEV_ADDR | I2C_WR);
				#endif

				/* ��3��������һ��ʱ�ӣ��ж������Ƿ���ȷӦ�� */
				if (i2c_WaitAck() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM����д��ʱ */
			}

			/* ��4���������ֽڵ�ַ��24C02ֻ��256�ֽڣ����1���ֽھ͹��ˣ������24C04���ϣ���ô�˴���Ҫ���������ַ */
			if (EE_ADDR_BYTES == 1)
			{
				i2c_SendByte((uint8_t)usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM������Ӧ�� */
				}
			}
			else
			{
				i2c_SendByte(usAddr >> 8);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM������Ӧ�� */
				}

				i2c_SendByte(usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM������Ӧ�� */
				}
			}
		}

		/* ��6������ʼд������ */
		/* _rease>0?0xFF:_pWriteBuf[i]  �Ļ����޸� */
		//i2c_SendByte(_rease>0?0xFF:_pWriteBuf[i]);
        i2c_SendByte(_rease>0?0x00:_pWriteBuf[i]);
		/* ��7��������ACK */
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM������Ӧ�� */
		}

		usAddr++;	/* ��ַ��1 */
	}

	/* ����ִ�гɹ�������I2C����ֹͣ�ź� */
	i2c_Stop();

	/* ͨ���������Ӧ��ķ�ʽ���ж��ڲ�д�����Ƿ����, һ��С�� 10ms
		CLKƵ��Ϊ200KHzʱ����ѯ����Ϊ30������
	*/
	for (m = 0; m < 1000; m++)
	{
		/* ��1��������I2C���������ź� */
		i2c_Start();

		/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */	
		#if EE_ADDR_A8 == 1											/* ����ط��Ĳ����д����� */
			i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));	/* �˴���дָ�� */
		#else		
			i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* �˴���дָ�� */
		#endif

		/* ��3��������һ��ʱ�ӣ��ж������Ƿ���ȷӦ�� */
		if (i2c_WaitAck() == 0)
		{
			break;
		}
	}
	if (m  == 1000)
	{
		goto cmd_fail;	/* EEPROM����д��ʱ */
	}

	/* ��д���� */
	//WP = 1;

	return 1;

cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	i2c_Stop();

	/* ��д���� */
	//WP = 1;

	return 0;
}


