
// �ó��� ��onenet������ �������޸� ���������Լ���RCT6���������У�ģ���ϴ���ʪ�� �Լ�����onenet����led ����

#include "bsp.h"



DHT11_T  g_DHT11;
uint16_t sysruntime=0;
float  ADCConvertedValueLocal[4]={0.0};
uint16_t g_nh3den=65;  //����Ũ��



time_t last_time=0;
time_t cur_time=0;


void res_update(time_t interval)
{
     
	    static uint8_t time=0;
       
       if(cur_time>=last_time+interval){  //���ʱ�䵽
            cur_time=0;
            last_time=0;				 
            sysruntime++;
			bsp_LedToggle(2);	 
            DHT11_ReadData(&g_DHT11);  //��ȡ��ʪ��
         //����������� X=16mA������Ϊ 0-100ppm,��
            //����Ũ�� C=[(16-4)/16]*��100-0�� =75ppm 
           
            ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;  //��ȡADת����ѹ
            g_nh3den = (uint16_t) (( (ADCConvertedValueLocal[0] * 10.0)-4) /16 *100); //��ȡ����Ũ��ֵ100ppm 
         
            if(time ==1)
                bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
            else if(time ==2)
                bsp_Diwen_Updatedata(0x0005,g_DHT11.Hum);
            else
            {
                bsp_Diwen_Updatedata(0x0003,g_nh3den);
                time =0;
            }

            time++;
		  
		
			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			
			   cur_time=nbiot_time();

			   
			} 	 

}	
int main( int argc, char *argv[])
{
    int life_time = 500; //ORIGINAL 1000
    int ret;
    uint8_t tempsec=0,tempmin=0;

    nbiot_init_environment( argc, argv );  

    bsp_ScreenChange(0);
    bsp_DelayMS(500);
    bsp_ScreenChange(0);
    bsp_DelayMS(500);
    g_page =0;
    FunO3Control(0,1);
    ee_ReadSaveData();
    printf("start while\r\n");
  
    while(1)
    {
             
            IWDG_Feed();

            res_update(2); 

            HC_Analyze();
            RTC_ReadClock();	/* ��ʱ�ӣ���������ȫ�ֱ��� g_tRTC */
 		   
            
            if(tempsec != g_tRTC.Sec)
            {
               if(g_tRTC.Sec %10 ==0)
               {
               //if(g_tRTC.Min % 10 ==1)
               printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
               }
               tempsec= g_tRTC.Sec;
            
               if((g_tRTC.Sec %2 ==0) &&(g_page !=0))
               {
                //   bsp_ScreenUPdateSYSSTATUS(g_status);
               }
               FunSwitch();
               
               bsp_LedToggle(1);

		       
            }
            
           
           
         
    } 


    return 0;
}
