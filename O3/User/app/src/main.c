
#include "bsp.h"			/* 底层硬件驱动 */
DHT11_T  g_DHT11;
//uint16_t temp ,humi;

nbiot_value_t temp;   /* 温度 */
nbiot_value_t humi;   /* 湿度 */

nbiot_value_t LED;
//int main(void)
//{
//    uint8_t keyvalue;
//    uint16_t  count=0;
//    uint8_t ret,flag=0;
//	bsp_Init();							/* 硬件初始化 */
//	while (1)
//	{
//      //  keyvalue = bsp_KeyScan(0);
//        keyvalue = bsp_MkeyScan();
//        if(keyvalue)
//        {
//            printf("keyvalue =%d\r\n",keyvalue);
//        }
//        if(keyvalue ==KEY1_PRES)  //K1 按下
//        {
//            flag ++ ;
//             printf("KEY1 PRESSED \r\n");
//             TIM_Cmd(TIM3,DISABLE);    
//             TIM_Cmd(TIM6,DISABLE);
//           
//             ret = DHT11_ReadData(&g_DHT11); //读取温湿度值
//             if (ret == 1)
//			{
//				printf("temp= %d  humi =  %d \r\n",g_DHT11.Temp, g_DHT11.Hum);
//			}
//			else
//			{
//				printf("\r\n未发现DHT22温湿度传感器\r\n");
//			}
//             TIM_Cmd(TIM3,ENABLE);
//             TIM_Cmd(TIM6,ENABLE);
////            bsp_IOToggle(1);
////            bsp_IOToggle(2);
////            bsp_IOToggle(3);
////            bsp_IOToggle(4);
//            if(flag%2 ==0)
//            {
//                printf("high\r\n");
//                bsp_IOOn(1);
//                bsp_IOOn(2);
//                bsp_IOOn(3);
//                bsp_IOOn(4);
//                bsp_LedOn(1);
//            }
//            else 
//            {
//                printf("low\r\n");
//                bsp_IOOff(1);
//                bsp_IOOff(2);
//                bsp_IOOff(3);
//                bsp_IOOff(4);
//                bsp_LedOff(1);
//            }
//        }
//        else if(keyvalue ==KEY2_PRES) //K2 按下
//        {
//            printf("KEY2 PRESSED \r\n");
//          //  DemoEEPROM();  //eeprom 测试程序
//        }
//         count++;
//         if(count%1000 ==0)//
//         tim_print_result();
//        if(count==2000)
//        {
//            count=0;
//        }
//         bsp_DelayMS(1);
//        
//	}
//}

void write_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
    printf( "write /%d/%d/%d：%d\r\n",
                  objid,
                  instid,
                  resid,data->value.as_bool );
    if(objid==3311&&instid==0&&resid==5850)
			// Led1_Set(data->value.as_bool);
        {
            if(data->value.as_bool)
            {
                bsp_LedOn(1);
            
            }
            else
                bsp_LedOff(1);
        }
		/*
		else if(objid==3301&&instid==0&&resid==5700){		    
		      illumi.flag |= NBIOT_UPDATED;
          BH1750_test();				 
				  illumi.value.as_float=result_lx;
		}else if(objid==3303&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};
		        temp.flag |= NBIOT_UPDATED;						 
            sht20=SHT20_GetValue();
				    temp.value.as_float=(int64_t)sht20.tempreture;
		
		}else if(objid==3304&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};
			      humi.flag |= NBIOT_UPDATED;					 
            sht20=SHT20_GetValue();
				    humi.value.as_float=(int64_t)sht20.humidity;
			
    }
		*/
}

void read_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{

		   
        if(objid==3301&&instid==0&&resid==5700){		    
              //    BH1750_test();				 
				//  illumi.value.as_float=result_lx;
		}else if(objid==3303&&instid==0&&resid==5700){
			     // SHT20_INFO sht20={0};					 
                //  sht20=SHT20_GetValue();
				//  temp.value.as_float=(int64_t)sht20.tempreture;
            temp.value.as_float=23.4;
		
		}else if(objid==3304&&instid==0&&resid==5700){
			    //  SHT20_INFO sht20={0};				 
                //  sht20=SHT20_GetValue();
				//  humi.value.as_float=(int64_t)sht20.humidity;
			
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
	    // SHT20_INFO sht20;
          if(cur_time>=last_time+interval){ 
            cur_time=0;
            last_time=0;				 
             
		    temp.flag |= NBIOT_UPDATED;		
           humi.flag |= NBIOT_UPDATED;					 
         //  sht20=SHT20_GetValue();
            temp.value.as_float=23.4;
            humi.value.as_float=45.6;
       
		   // illumi.flag |= NBIOT_UPDATED;
           // BH1750_test();				 
			//illumi.value.as_float=result_lx;
		/*
            press.flag |= NBIOT_UPDATED;		
            altitude.flag |= NBIOT_UPDATED;	
            BMP180_test();
            press.value.as_float=bmp180.p;
            altitude.value.as_float=bmp180.altitude;

            acceleX.flag |= NBIOT_UPDATED;		
            acceleY.flag |= NBIOT_UPDATED;
            acceleZ.flag |= NBIOT_UPDATED;							
            ADXL345_GetValue();
            acceleX.value.as_float=adxlInfo.incidence_Xf;
            acceleY.value.as_float=adxlInfo.incidence_Yf;
            acceleZ.value.as_float=adxlInfo.incidence_Zf;
         */
			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			
			   cur_time=nbiot_time();
			   
			} 	 

}	
int main( int argc, char *argv[] )
{
      int life_time = 1000;
	  int ret;
      nbiot_init_environment( argc, argv );  
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
                                  &LED );//灯控制
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
                                  &temp );//温度
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
                                  &humi );//湿度
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(humi) failed, code = %d.\r\n", ret );
        }

//        illumi.type = NBIOT_FLOAT;
//        illumi.flag = NBIOT_READABLE;
//        ret = nbiot_resource_add( dev,
//                                  3301,
//                                  0,
//                                  5700,
//                                  &illumi );//光照
//        if ( ret )
//        {
//            nbiot_device_destroy( dev );
//            printf( "device add resource(illumi) failed, code = %d.\r\n", ret );
//        }
				/*
        press.type = NBIOT_FLOAT;
        press.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  3,
                                  5600,
                                  &press );//压力
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(press) failed, code = %d.\r\n", ret );
        }
				
        altitude.type = NBIOT_FLOAT;
        altitude.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  4,
                                  5600,
                                  &altitude );//海拔
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(temp) failed, code = %d.\r\n", ret );
        }
				
        acceleX.type = NBIOT_FLOAT;
        acceleX.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  5,
                                  5600,
                                  &acceleX );//x轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleX) failed, code = %d.\r\n", ret );
        }
				acceleY.type = NBIOT_FLOAT;
        acceleY.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  6,
                                  5600,
                                  &acceleY );//y轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleY) failed, code = %d.\r\n", ret );
        }
				acceleZ.type = NBIOT_FLOAT;
        acceleZ.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  7,
                                  5600,
                                  &acceleZ );//z轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleZ) failed, code = %d.\r\n", ret );
        }
				*/
	    nbiot_object_add(dev);
        ret = nbiot_device_connect(dev,100);

        if ( ret )
        {
            printf( "connect OneNET failed.\r\n" );
			nbiot_reset();
        }else{
					 // Led4_Set(LED_ON);
	         printf( "connect OneNET success.\r\n" );
				 
				}
    do
    {
             ret = nbiot_device_step( dev, 1);
             if ( ret )
             {
               printf( "device step error, code = %d.\r\n", ret );
				     //  Led4_Set(LED_OFF);
               printf( "connect server failed.\r\n" );
		       nbiot_reset();
             }else{ 
               res_update(200);	
             }					 
			      
    } while(1);
    nbiot_clear_environment();
  //  getchar();
    printf( "press enter key to exit..." );

    return 0;
}








