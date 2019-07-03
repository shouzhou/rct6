
// �ó��� ��onenet������ �������޸� ���������Լ���RCT6���������У�ģ���ϴ���ʪ�� �Լ�����onenet����led ����

#include "bsp.h"

nbiot_value_t temp;   /* �¶� */
nbiot_value_t humi;   /* ʪ�� */
nbiot_value_t illumi; //����
nbiot_value_t LED;

DHT11_T  g_DHT11;
uint16_t sysruntime=0;
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


void res_update(time_t interval)
{
     // if((cur_time%10 ==0))
      {
           tim_print_result(); 
      }
	    
       if(cur_time>=last_time+interval){  //���ʱ�䵽
            cur_time=0;
            last_time=0;				 
            sysruntime++;
		    temp.flag |= NBIOT_UPDATED;		  //������Ҫ����
            humi.flag |= NBIOT_UPDATED;					 
            DHT11_ReadData(&g_DHT11);  //��ȡ��ʪ��

            temp.value.as_float=(float)g_DHT11.Temp/10.0;  //ת��Ϊ������
			humi.value.as_float=(float)g_DHT11.Hum/10.0;
           
            bsp_Diwen_Updatedata(0x0003,g_DHT11.Temp);
            bsp_Diwen_Updatedata(0x0004,g_DHT11.Hum);
            bsp_Diwen_Updatedata(0x0005,300);
            bsp_Diwen_Updatedata(0x0006,sysruntime/6);
       
		    illumi.flag |= NBIOT_UPDATED;
     	    illumi.value.as_float=300.0;
		
			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			
			   cur_time=nbiot_time();
            //   printf("cur_time = %d \r\n",(uint16_t)cur_time);
			   
			} 	 

}	
int main( int argc, char *argv[])
{
       int life_time = 500; //ORIGINAL 1000
	   int ret;
       nbiot_init_environment( argc, argv );  
  if(g_WithoutOnenet == 0)  //ʹ��ONENET
  {
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

            illumi.type = NBIOT_FLOAT;
            illumi.flag = NBIOT_READABLE;
            ret = nbiot_resource_add( dev,
                                      3301,
                                      0,
                                      5700,
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
    while(1)
    {
             IWDG_Feed();
            if(g_WithoutOnenet == 0)
            {
                 ret = nbiot_device_step( dev, 1);
                 if ( ret )  //fail
                 {
                   printf( "device STEP error, code = %d.\r\n", ret );
                   g_FailTime= g_FailTime +1;
                   eeResult[7] = g_FailTime;
                   ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
                          // Led4_Set(LED_OFF);
                   printf( "connect server failed.\r\n" );
                         nbiot_reset();
                 }else{ 
                        res_update(180);
                       if(g_FailTime>0)
                       {
                        g_FailTime= 0;
                        eeResult[7] = g_FailTime;
                        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //����ʧ�ܴ���
                       }                       
                 }	
             
           }
             HC_Analyze();
             RTC_ReadClock();	/* ��ʱ�ӣ���������ȫ�ֱ��� g_tRTC */
		
		   /* ��ӡʱ�� */
            // if(g_tRTC.Sec %10 ==0)
		   printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, 
			  g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);

			      
    } 
    nbiot_clear_environment();
 
    printf( "press enter key to exit..." );

    return 0;
}
