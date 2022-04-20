
// �ó��� ��onenet������ �������޸� ���������Լ���RCT6���������У�ģ���ϴ���ʪ�� �Լ�����onenet����led ����

#include "bsp.h"

nbiot_value_t temp;   /* �¶� */
nbiot_value_t humi;   /* ʪ�� */
nbiot_value_t illumi; //����
nbiot_value_t LED;
nbiot_value_t setvalue;

DHT11_T  g_DHT11;
 //char test[20] = {'b','a','a','a','a','b','b','c'};
 char test[50] ="hello world123";
void write_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
    printf( "write /%d/%d/%d��%d\r\n",
                  objid,
                  instid,
                  resid,data->value.as_bool );
    if(objid==3311&&instid==0&&resid==5850)
    {
			 if(data->value.as_bool)
             {
                 bsp_LedOn(1);
             }
             else
             {
                 bsp_LedOff(1);
             }
    }
}

void read_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
       
		   
        if(objid==3301&&instid==0&&resid==5700){		    
                		 
				  illumi.value.as_float=300.0;
		}else if(objid==3303&&instid==0&&resid==5700){
                
                  DHT11_ReadData(&g_DHT11);

				  temp.value.as_float=(float)g_DHT11.Temp/10.0;
             
		
		}else if(objid==3304&&instid==0&&resid==5700){
                  DHT11_ReadData(&g_DHT11);
				  humi.value.as_float=(float)g_DHT11.Hum/10.0;
           			
    }
	
}

void execute_callback( uint16_t       objid,
                       uint16_t       instid,
                       uint16_t       resid,
                       nbiot_value_t *data,
                       const void    *buff,
                       size_t         size )
{
    printf( "execute /%d/%d/%d\r\n",
                  objid,
                  instid,
                  resid );
}

#ifdef NOTIFY_ACK
void notify_ack_callback( uint16_t       objid,
                          uint16_t       instid,
                          uint16_t       resid,
                          nbiot_value_t *data,
                          bool           ack )
{
    printf( "notify ack /%x/%x/%x(%s)\r\n",
                  objid,
                  instid,
                  resid,
                  ack ? "true":"false" );
}
#endif

char uri[] = "coap://183.230.40.40:5683";
nbiot_device_t *dev = NULL;


time_t last_time=0;
time_t cur_time=0;

void res_updateWithoutnet(time_t interval)
{
       if(cur_time>=last_time+interval){  //���ʱ�䵽
            cur_time=0;
            last_time=0;				 
            DHT11_ReadData(&g_DHT11);  //��ȡ��ʪ��
            bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
            bsp_DelayMS(5);
            bsp_Diwen_Updatedata(0x0005,g_DHT11.Hum);
            bsp_DelayMS(5);
            bsp_Diwen_Updatedata(0x0006,g_sysruntime/6);
 			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			   cur_time=nbiot_time();
			} 	 

}	
void res_update(time_t interval)
{
     
	  
       if(cur_time>=last_time+interval){  //���ʱ�䵽
            cur_time=0;
            last_time=0;				 
            //sysruntime++;
		    temp.flag |= NBIOT_UPDATED;		  //������Ҫ����
            humi.flag |= NBIOT_UPDATED;	
            illumi.flag |= NBIOT_UPDATED ;
            sprintf(test,"%02d--%02d--%6.1f--%6.1f--%6.1f ",g_plant,g_chemical,kp,kv,kc);
            illumi.value.as_buf.val = test;
            illumi.value.as_buf.len = sizeof(test);
            DHT11_ReadData(&g_DHT11);  //��ȡ��ʪ��

            temp.value.as_float=(float)g_DHT11.Temp/10.0;  //ת��Ϊ������
			humi.value.as_float=(float)g_DHT11.Hum/10.0;
           
            bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
            bsp_DelayMS(5);
            bsp_Diwen_Updatedata(0x0005,g_DHT11.Hum);
            bsp_DelayMS(5);
            bsp_Diwen_Updatedata(0x0006,g_sysruntime/6);
            bsp_DelayMS(5);
            bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
	
			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			
			   cur_time=nbiot_time();
			   
			} 	 

}	
void OneNetCreateRes(void)
{
    int life_time = 500; //ORIGINAL 1000
	int ret;
#ifdef NOTIFY_ACK
    ret = nbiot_device_create( &dev,
                               life_time,
                               write_callback,
                               read_callback,
                               execute_callback,
                               notify_ack_callback );
#else
    ret = nbiot_device_create( &dev,
                               life_time,
                               write_callback,
                               read_callback,
                               execute_callback );
#endif
   
    if ( ret )
    {
        nbiot_device_destroy( dev );
        printf( "device add resource(/3200/0/5750) failed, code = %d.\r\n", ret );
    }
        LED.type = NBIOT_BOOLEAN;
        LED.flag = NBIOT_READABLE|NBIOT_WRITABLE;
        ret = nbiot_resource_add( dev,
                                  3311,
                                  0,
                                  5850,
                                  &LED );//�ƿ���
    if ( ret )
    {
        nbiot_device_destroy( dev );
        printf( "device add resource(LED) failed, code = %d.\r\n", ret );
    }
            
        temp.type = NBIOT_FLOAT;
        temp.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3303,
                                  0,
                                  5700,
                                  &temp );//�¶�
    if ( ret )
    {
        nbiot_device_destroy( dev );
        printf( "device add resource(temp) failed, code = %d.\r\n", ret );
    }
            
            
        humi.type = NBIOT_FLOAT;
        humi.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3304,
                                  0,
                                  5700,
                                  &humi );//ʪ��
    if ( ret )
    {
        nbiot_device_destroy( dev );
        printf( "device add resource(humi) failed, code = %d.\r\n", ret );
    }

        illumi.type = NBIOT_STRING;
        illumi.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3301,
                                  0,
                                  5701,
                                  &illumi );//����
    if ( ret )
    {
        nbiot_device_destroy( dev );
        printf( "device add resource(illumi) failed, code = %d.\r\n", ret );
    }

    nbiot_object_add(dev);
    ret = nbiot_device_connect(dev,100);

    if ( ret )
    {
        printf( "device CONNECT error, code = %d.\r\n", ret );
        printf( "connect OneNET failed.\r\n" );
        g_FailTime= g_FailTime +5;
        eeResult[7] = g_FailTime;
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
              nbiot_reset();
    }else{
        printf( "connect OneNET success.\r\n" );
        g_FailTime= 0;
        eeResult[7] = g_FailTime;
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //����ʧ�ܴ���
        IWDG_Init(5,1250); //4S ���Ź�
          } 
}
void OneNetCheck(void)
{
           int ret;
           uint8_t oktime=0;
            while(1)
            {
                 ret = nbiot_device_step( dev, 0);
                 if ( ret )  //fail
                 {
                   printf( "NET CHECK error, code = %d.\r\n", ret );
                   g_FailTime= g_FailTime +1;
                   eeResult[7] = g_FailTime;
                   ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
                   // Led4_Set(LED_OFF);
                   printf( "NET CHECK failed.\r\n" );
                         nbiot_reset();
                 }else
                 { 
                       
                       printf( "NET CHECK succ %d.\r\n",oktime );
                       if(g_FailTime>0)
                       {
                        g_FailTime= 0;
                        eeResult[7] = g_FailTime;
                        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //����ʧ�ܴ���
                       } 
                        oktime++;                       
                       if(oktime>3) break;
                 }	
             
           }
}
int main( int argc, char *argv[])
{
   
    int ret;
    uint8_t tempsec=0;
    float  ADCConvertedValueLocal[4]={0.0};
   
    
    nbiot_init_environment( argc, argv );  
    
    if(g_WithoutOnenet == 0)  //ʹ��ONENET
     {
         OneNetCreateRes();  //������Դ
     }
    if(g_WithoutOnenet == 0)  //ʹ��ONENET
     {
         OneNetCheck();  //ȷ���Ѿ����������� 
     }
     bsp_SysCheck();//ϵͳ�Լ죬������ɹ����򵯳������Ի���
     BASIC_TIM_Init();//�漰���� �Լ��������� �����ϵͳ�ȶ��� �ٿ���
    // g_status = 31;  //���������Լ� ���ڲ���
     g_status = SYSINIFINISH; //��ʼ�����
     g_touch = 13; //���½���
    while(1)
    {
       // printf("while(1)\r\n");
        IWDG_Feed();
        if(g_WithoutOnenet == 0)  // use  onenet platform
        {
         ret = nbiot_device_step( dev, 0);
         if ( ret )  //fail
         {
           printf( "device STEP error, code = %d.\r\n", ret );
           g_FailTime= g_FailTime +1;
           eeResult[7] = g_FailTime;
           ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
           printf( "connect server failed.\r\n" );
           //  9();
           if(g_FailTime >10) 
           {
               g_WithoutOnenet = 1; //ʧ��10������ ��ֱ�ӽ�������
               bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
           }
         }else
         { 
               res_update(NETUPDATETIME);
               if(g_FailTime>0)
               {
                g_FailTime= 0;
                eeResult[7] = g_FailTime;
                ee_WriteBytes(0,&eeResult[7],0x07,0x01); //����ʧ�ܴ���
               }                       
         }	

        }
        else  //dont use onenet platform just show on the screen
        {
        res_updateWithoutnet(NONETUPDATETIME);
        }
        //    tim_print_result(); 
        //    HC_Analyze();
        RTC_ReadClock();	/* ��ʱ�ӣ���������ȫ�ֱ��� g_tRTC */
        //  bsp_ScreenUpdateText();
        FunTouch();
        if(tempsec != g_tRTC.Sec)
        {
        if(g_tRTC.Sec%10 ==0)
        printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day,g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
        tempsec=g_tRTC.Sec;
        //  if(g_tRTC.Sec %10 ==0)
        //  bsp_ScreenUpdateText();
        FunSwitch();

        #ifdef USEDEBUG
        //               ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;
        //               printf(" \r\n ͨ��10(PC0)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[0]);
        //               ADCConvertedValueLocal[1]=(float)ADCConvertedValue[1]/4096*3.3;
        //               printf(" \r\n ͨ��11(PC1)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[1]);
        //               ADCConvertedValueLocal[2]=(float)ADCConvertedValue[2]/4096*3.3;
        //               printf(" \r\n ͨ��12(PC2)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[2]);
        //               ADCConvertedValueLocal[3]=(float)ADCConvertedValue[3]/4096*3.3;
        //               printf(" \r\n ͨ��13(PC3)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[3]);
        #endif
        }
           
		       
    } 

}
