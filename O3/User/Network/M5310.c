#include "bsp.h"
//#include "led.h"
extern int8_t dl_buf_id;
static char cmd_buff[1024];


void hex_to_hexstring(char *src,char *dst,int len)
{
    int i = 0,j = 0;
    for(i=0,j=0;j<len*2;i++,j++)
    {
      if((src[i]<=9)&&(src[i]>=0))
	    {
				dst[j] =  '0';
		    dst[++j] = src[i] + '0';	
      }
      else  if(src[i]>=0x0A&&src[i]<=0x0F)
      {
        dst[j] =  '0';
		    dst[++j] = src[i] + 55;	
      }
      else if (src[i]>=0x0F)
      {
				dst[j] =  src[i]/16 +'0';
				switch (dst[j])
				{
                case '0': dst[j] = '0';break;
                case '1': dst[j] = '1';break;
                case '2': dst[j] = '2';break;
                case '3': dst[j] = '3';break;
                case '4': dst[j] = '4';break;
                case '5': dst[j] = '5';break;
                case '6': dst[j] = '6';break;
                case '7': dst[j] = '7';break;
                case '8': dst[j] = '8';break;
                case '9': dst[j] = '9';break;
                case ':': dst[j] = 'A';break;
                case ';': dst[j] = 'B';break;
                case '<': dst[j] = 'C';break;
                case '=': dst[j] = 'D';break;
                case '>': dst[j] = 'E';break;
                case '?': dst[j] = 'F';break;
                default : break;
				}
				dst[++j] =  src[i]%16 +'0';
				switch (dst[j])
				{
                case '0': dst[j] = '0';break;
                case '1': dst[j] = '1';break;
                case '2': dst[j] = '2';break;
                case '3': dst[j] = '3';break;
                case '4': dst[j] = '4';break;
                case '5': dst[j] = '5';break;
                case '6': dst[j] = '6';break;
                case '7': dst[j] = '7';break;
                case '8': dst[j] = '8';break;
                case '9': dst[j] = '9';break;
                case ':': dst[j] = 'A';break;
                case ';': dst[j] = 'B';break;
                case '<': dst[j] = 'C';break;
                case '=': dst[j] = 'D';break;
                case '>': dst[j] = 'E';break;
                case '?': dst[j] = 'F';break;
                default : break;
					}
				}
               
				}	 
		 strcat(dst,",1,1\r\n");
		// nbiot_printf(dst);
		 return;
}


uint32_t ip_SendData(int8_t * buf, uint32_t len)
{
     SentData(buf,"OK",100);
     return len;
}

void netif_rx(uint8_t*buf,uint16_t *read)
{
     uint8_t *msg_p=NULL;
     uint8_t ptr[1024]={0};
     *read=fifo_get(dl_buf_id,ptr);
     if(*read!=0)
     {
        if((msg_p= strstr((const char *)ptr, "+MIPL"))!=NULL)
        {              
           memcpy(buf,ptr,*read);
        }else{
        
           *read=0;
        }
     }
       
}
void M5310_Power_Init(void)
{
//        GPIO_InitTypeDef  GPIO_InitStructure;

//        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PE端口时钟

//        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LED0-->PB.9 端口配置
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
//        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
//        GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
//        GPIO_SetBits(GPIOC,GPIO_Pin_4);						 //PB.5 输出高
//        //GPIO_SetBits(GPIOE,GPIO_Pin_5); 						 //PE.5 输出高 
}

void netdev_init(void)
{
        SendCmd("AT+NRB\r\n", "OK", 5000,0,10); 
        SendCmd("AT+CIMI\r\n", "OK", 2000,3,5);
        // SendCmd("AT+CGMR\r\n", "OK", 5000,0,10);
        SendCmd("AT+CMVER\r\n", "OK", 5000,0xFFFF,10);//检测M5310的固件版本是否为sp3以上
        SendCmd("AT+CMEE=1\r\n","OK", 2000,0,10);
        SendCmd("AT+CSCON=1\r\n","OK", 2000,0,10);  
        SendCmd("AT+CEREG=2\r\n","OK", 2000,0,10); 
        SendCmd("AT+CEDRXS=0,5\r\n","OK", 3000,0,5); 
        SendCmd("AT+CPSMS=0\r\n","OK", 3000,0,5); 
        //   SendCmd("AT+CSCON?\r\n","CSCON:1,1", 5000); 
        SendCmd("AT+CEREG?\r\n","CEREG:2,1", 3000,1000,5); 
        SendCmd("AT+csq\r\n", "OK", 2000,0,5); 
        SendCmd("AT+NUESTATS\r\n", "OK", 2000,0,5); 
        //   SendCmd("AT+CGDCONT?\r\n", "OK", 5000,0,10);
        printf("connect NB-IoT sucess\r\n");
#ifndef HARDWARE_TEST
	  // Led3_Set(LED_ON);
      
#else	
        Beep_test(200);
        Beep_test(200);
#endif		
} 

void usage( const char *name )
{
	printf( "Usage: %s [OPTION]\r\n", name );
	printf( "Launch a client.\r\n" );
	printf( "version: v1.2\r\n" );
	printf( "Options:\r\n" );
	printf( "-b BOOT\t\tSet the bootstrap mode of the client.Default: 0\r\n" );
	printf( "-d DEBUG\tSet the debug mode of the client.Default: 0\r\n" );
	printf( "-e ENCRYPT\tSet the encrypt of the client.\r\n" );
	printf( "-i URI\t\tSet the coap uri of the server to connect to. For example: coap://localhost:5683\r\n" );
	printf( "-n NAME\t\tSet the endpoint name[imei;imsi] of the client.\r\n" );
	printf( "-p PORT\t\tSet the local port of the client to bind to. Default: srand\r\n" );
	printf( "-t TIME\t\tSet the lifetime of the client. Default: 300\r\n" );
	printf( "-u BLOCK1\tSet COAP option BLOCK1(PUT or POST),0-6. Default 5(512B),2^(4+n)\r\n");
	printf( "-g BLOCK2\tSet COAP option BLOCK2(GET),0-6. Default 5(512B),2^(4+n)\r\n");
	printf( "-x BLOCK2TH\tSet max size to trigger block-wise operation,0-2. Default 2(1024B),2^(8+n)\r\n");
	printf( "\r\n" );
}

void output_buffer1(unsigned char *buffer, int length,int index, int flag)
{
	int i = 0;
	while ( i < length )
	{
		printf( "%02X", buffer[i++] );
	}
	printf(",%d,%d\r\n",index,flag);
}
/*****************************************************************
       0               1               2               3 
0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| boot | encrypt|     debug     |          local_port         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          life_time                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           host_len            |             host            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          ep_name_len          |      ep_name(imei;imsi)     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  coap_block1  |  coap_block2  |    block2th   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*****************************************************************/
void mipl_generate(char *buf, size_t buflen, MIPL_T *mipl)
{
	strcpy(buf,"AT+MIPLCREATE=49,130031F10003F2002304001100000000000000123138332E3233302E34302E34303A35363833000131F30008C000000000,0,49,0\r\n"); //更改到适配的平台
	SendCmd(buf,"+MIPLCREATE:0",300,0,300);
	
	/*uint32_t offset = 0;
	int mipl_num = 0;
	char mode,temp[5];
	memset(buf, 0, buflen);

	mode = ((mipl->boot & 0x1) << 4) | (mipl->encrypt & 0x1);
	memcpy(buf + offset, &mode, 1);
	offset++;
	memcpy(buf + offset, &mipl->debug, 1);
	offset++;
	memcpy(buf + offset, &mipl->port, 2);
	offset += 2;
	memcpy(buf+offset, &mipl->keep_alive, 4);
	offset += 4;

	memcpy(buf + offset, &mipl->uri_len, 2);
	offset += 2;
	memcpy(buf + offset, mipl->uri, mipl->uri_len);
	offset += mipl->uri_len;
	memcpy(buf + offset, &mipl->ep_len, 2);
	offset += 2;
	memcpy(buf + offset, mipl->ep, mipl->ep_len);
	offset += mipl->ep_len;

  *(buf + offset) = mipl->block1;
	offset++;
	*(buf + offset) = mipl->block2;
	offset++;
	*(buf + offset) = mipl->block2th;
	offset++;
	
	mipl_num = 0;
	while(offset > 512)//??+MIPLCONF??????1024??????
	{
		printf("MIPLCONF is too long\r\n");
		return ;
	}
	
	strcpy(cmd_buff,"AT+MIPLCONF=");
    nbiot_itoa(offset,temp,5);
	strcat(cmd_buff,temp);
	strcat(cmd_buff,",");
	hex_to_hexstring(buf,cmd_buff+strlen(cmd_buff),offset);
	SendCmd(cmd_buff,"OK",300,0,10);
	//output_buffer1( (unsigned char *)(buf + (mipl_num >> 10)),offset,mipl_num + 1,1);//??
	*/
}

void init_miplconf(u32 lifetime,const char *uri,const char *ep)
{
	MIPL_T mipl;
	char buffer[512];
	mipl.boot=MIPL_BOOT;
	mipl.encrypt=MIPL_ENCRYPT;
	mipl.debug=MIPL_DEBUG;
	mipl.port=MIPL_PORT;
	mipl.keep_alive=lifetime;
	mipl.uri=uri;
	mipl.uri_len=strlen(uri);
	mipl.ep=ep;
	mipl.ep_len=strlen(ep);
	mipl.block1 = MIPL_BLOCK1,	//COAP option BLOCK1(PUT or POST),0-6. 2^(4+n)  bytes
	mipl.block2 = MIPL_BLOCK2,	//COAP option BLOCK2(GET),0-6. 2^(4+n)  bytes
	mipl.block2th = MIPL_BLOCK2TH,
	mipl_generate(buffer,sizeof(buffer),&mipl);
}


void m5310_addobj(uint16_t	   objid,
  	              uint8_t	   instcount,
                  uint8_t     *bitmap,
                  uint8_t      attrs,
                  uint8_t      acts)
{
	char tmp[10];
	memset(cmd_buff,0,50);
	memcpy(cmd_buff,"AT+MIPLADDOBJ=0,",sizeof("AT+MIPLADDOBJ=0,"));
	nbiot_itoa(objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(instcount,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,bitmap);
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,",");
	
  nbiot_itoa(attrs,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
  nbiot_itoa(acts,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,"\r\n");
	SendCmd(cmd_buff,"OK",300,0,5);
}

void m5310_delobj(uint16_t  objid)
{
	 char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLDELOBJ=0,",sizeof("AT+MIPLDELOBJ=0,"));
	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 SendCmd(cmd_buff,"OK",300,0,5);
 }


 size_t m5310_register_request( uint8_t  *buffer,									    
                                size_t    buffer_len,
                                uint16_t  lifetime,	                  
                                uint8_t   waittime)
{   
	char status=0;
	char ative[6];
  nbiot_itoa(lifetime,ative,6);
	memcpy(buffer,"AT+MIPLOPEN=0,",sizeof("AT+MIPLOPEN=0,"));
	strcat(buffer,ative);
	strcat(buffer,",");
	nbiot_itoa(waittime,ative,6);
	strcat(buffer,ative);
	strcat(buffer,"\r\n");
	status=SendCmd(buffer,"OK",300,0,5);
	if(status==2)
		SendCmd(buffer,"OK",300,0,5); 
	return buffer_len;
}

 size_t m5310_register_update (uint16_t lifttime, 
                               uint8_t  withobj,  
                               uint8_t *buffer,									    
                               size_t  buffer_len)
{   
  size_t  len=0;
	char ative[6];
  nbiot_itoa(lifttime,ative,6);
	memcpy(buffer,"AT+MIPLUPDATE=0,",sizeof("AT+MIPLUPDATE=0,"));
	strcat(buffer,ative);
	strcat(buffer,",");
	nbiot_itoa(withobj,ative,1);
	strcat(buffer,ative);
	strcat(buffer,"\r\n");
	len=strlen(buffer)+1;
	if(len<buffer_len){
	 SendCmd(buffer,"OK",300,0,5);
	 return len;
   }
	return 0;
}
 size_t m5310_close_request( uint8_t  *buffer,									    
                             size_t    buffer_len)
{   
    size_t  len=0;
	len=strlen("AT+MIPLCLOSE=0\r\n")+1;
	if(len<buffer_len){
	 memcpy(buffer,"AT+MIPLCLOSE=0\r\n",len);
	 SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5);
	 return len;
   }
	return 0;
}  


 void m5310_notify_upload(const nbiot_uri_t *uri,uint8_t type,char *data,uint8_t flag,uint8_t index,uint16_t ackid)
{
	
  char tmp[10];
	memset(cmd_buff,0,sizeof(cmd_buff));
	memcpy(cmd_buff,"AT+MIPLNOTIFY=0,0,",sizeof("AT+MIPLNOTIFY=0,0,"));
	nbiot_itoa(uri->objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri->instid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri->resid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
    nbiot_itoa(type,tmp,1);
    strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	nbiot_itoa(strlen(data),tmp,10);
    strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,data);
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,",");
	
	nbiot_itoa(index,tmp,1);
    strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
    nbiot_itoa(flag,tmp,1);
	strcat(cmd_buff,tmp);
#ifdef NOTIFY_ACK
if(flag==0){
	strcat(cmd_buff,",");
	nbiot_itoa(ackid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,"\r\n");
}else{

   strcat(cmd_buff,"\r\n");

}	
	
#else

    strcat(cmd_buff,"\r\n");

#endif
    printf("send data:");
    printf("%s\r\n",cmd_buff);
	  SentData(cmd_buff,"OK",100);
}

 void m5310_read_upload(const nbiot_uri_t *uri,uint8_t type,char *data,uint16_t msgid,uint8_t result,uint8_t index,uint8_t flag)
{
	
  char tmp[10];
	memset(cmd_buff,0,sizeof(cmd_buff));
	memcpy(cmd_buff,"AT+MIPLREADRSP=0,",sizeof ("AT+MIPLREADRSP=0,"));
	nbiot_itoa(msgid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	nbiot_itoa(result,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	nbiot_itoa(uri->objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri->instid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri->resid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
    nbiot_itoa(type,tmp,1);
   strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	nbiot_itoa(strlen(data),tmp,10);
    strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,data);
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,",");
	
	nbiot_itoa(index,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	
   nbiot_itoa(flag,tmp,1);
	strcat(cmd_buff,tmp);
   strcat(cmd_buff,"\r\n");
	printf("read rsp:");
  printf("%s\r\n",cmd_buff);
	SentData(cmd_buff,"OK",100);
}

void m5310_write_rsp(int suc,uint16_t msgid)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
   memcpy(cmd_buff,"AT+MIPLWRITERSP=0,",sizeof("AT+MIPLWRITERSP=0,"));
	 nbiot_itoa(msgid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 nbiot_itoa(suc,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 printf("write rsp:");
   printf("%s\r\n",cmd_buff);
	 SentData(cmd_buff,"OK",100);
  
}

void m5310_execute_rsp(int suc,uint16_t msgid)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLEXECUTERSP=0,",sizeof("AT+MIPLEXECUTERSP=0,"));
	 nbiot_itoa(msgid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 nbiot_itoa(suc,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 printf("execute rsp:");
   printf("%s\r\n",cmd_buff);
   SentData(cmd_buff,"OK",100);
  
}

void m5310_discover_rsp(uint16_t objid,char *resid)
{
     char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLDISCOVERRSP=0,",sizeof("AT+MIPLDISCOVERRSP=0,"));
	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",1,");
	
	 nbiot_itoa(strlen(resid)-2,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	
	 strcat(cmd_buff,resid);
	 strcat(cmd_buff,"\r\n");
	 printf("discover rsp:");
   printf("%s\r\n",cmd_buff);
   SentData(cmd_buff,"OK",100);
  
}

  size_t m5310_delete_request( uint8_t  *buffer,									    
                             size_t    buffer_len)
{   
    size_t  len=0;
	len=strlen("AT+MIPLDEL=0\r\n")+1;
	if(len<buffer_len){
	 memcpy(buffer,"AT+MIPLDEL=0\r\n",len);
	 SendCmd("AT+MIPLDEL=0\r\n","OK",300,0,5);
	 return len;
   }
	return 0;
}  

