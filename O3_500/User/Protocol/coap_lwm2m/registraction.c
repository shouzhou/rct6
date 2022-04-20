/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"
#include "M5310.h"
//#include "led.h"

#define REGISTER_START  1
#define REGISTER_UPDATE 2
#define DEREGISTER      3



#ifdef NOTIFY_ACK
static void registraction_reply( nbiot_device_t    *dev,
                                      bool          suc,
                                      bool          ack,
                                      uint16_t      ackid )
#else
static void registraction_reply( nbiot_device_t *dev,
                                 bool            suc,
                                 bool            ack )
#endif
{
    if ( dev->state == STATE_REG_PENDING )
    {
        dev->registraction = nbiot_time();	
        if ( suc == true )
        {
            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_start( nbiot_device_t *dev,
	                        int            timeout,
	                        uint8_t        *buffer,
                          size_t          buffer_len)
{
	    size_t          length=0;
#ifdef NBIOT_BOOTSTRAP
    if ( dev->state == STATE_BS_FINISHED )
#else
    if ( dev->state == STATE_REG_FAILED ||
         dev->state == STATE_DEREGISTERED )
#endif
    {
      length=m5310_register_request(buffer,buffer_len,dev->life_time,timeout);
		 if(length>0){
		 	dev->state = STATE_REG_PENDING;
#ifdef NOTIFY_ACK
		    nbiot_transaction_add( dev,
			                       true,
								   buffer,
								   length,
								   registraction_reply,
								   NULL );
#else
		  nbiot_transaction_add( dev,
								true,
								buffer,
								length,
								registraction_reply );
#endif

	   }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
		
    }

    return COAP_NO_ERROR;
}

#ifdef NOTIFY_ACK
static void registraction_update_reply( nbiot_device_t    *dev,
                                        bool               suc,
                                        bool               ack,
                                        uint16_t           ackid )
#else
static void registraction_update_reply( nbiot_device_t *dev,
                                        bool            suc,
                                        bool            ack )
#endif
{
    if ( dev->state == STATE_REG_UPDATE_PENDING )
    {
        dev->registraction = nbiot_time();
        if (suc == true)
        {
            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_update( nbiot_device_t *dev,
	                         uint8_t        *buffer,
                           size_t          buffer_len)
{
    size_t          length=0;
  //  if (dev->state == STATE_REG_UPDATE_NEEDED )
    {
			   printf("update\r\n");
         length=m5310_register_update(dev->life_time,0,buffer,buffer_len);
		    if(length>0){
		 	  dev->state = STATE_REG_UPDATE_PENDING;

#ifdef NOTIFY_ACK
			   nbiot_transaction_add( dev,
			                          true,
									              buffer,
									              length,
									              registraction_update_reply,
									              NULL );
#else
			   nbiot_transaction_add( dev,
			                          true,
									             buffer,
									             length,
									             registraction_update_reply );
#endif

	   }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
		
    }

    return COAP_NO_ERROR;
}

#ifdef NOTIFY_ACK
static void deregister_reply(nbiot_device_t    *dev,
                             bool               suc,
                             bool               ack,
                             uint16_t           ackid )
#else
static void deregister_reply( nbiot_device_t    *dev,
                              bool               suc,
                              bool               ack)
#endif
{
    if ( dev->state == STATE_DEREG_PENDING )
    {
        dev->state = STATE_DEREGISTERED;
    }
}

int nbiot_deregister( nbiot_device_t *dev,
                      uint8_t        *buffer,
                      size_t          buffer_len )

{
     size_t          length=0;
  /*  if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED ||
         dev->state == STATE_REG_UPDATE_PENDING ||
         dev->state == STATE_REG_FAILED)*/
    {
		length=m5310_close_request(buffer,buffer_len);
		 if(length>0){
           dev->state = STATE_DEREG_PENDING;
/*		   
#ifdef NOTIFY_ACK
			   nbiot_transaction_add( dev,
			                          true,
									  buffer,
									  length,
									  deregister_reply,
									  NULL );
#else
			   nbiot_transaction_add( dev,
			                          true,
									  buffer,
									  length,
									  deregister_reply);
#endif
*/

	   }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
    }

    return COAP_NO_ERROR;
}

void nbiot_register_step( nbiot_device_t *dev,
                                time_t         now,
                                uint8_t        *buffer,
                                 size_t        buffer_len )
{
     int next_update= dev->life_time;
    if ( dev->state == STATE_REGISTERED )
    {
          next_update = next_update-50;
        if (dev->registraction + next_update <= now)
        {
            nbiot_register_update(dev,buffer,buffer_len);
        }
    }
     if (dev->state == STATE_REG_UPDATE_NEEDED)
     {
            nbiot_register_update(dev,buffer,buffer_len);
     }
		if ( dev->state == STATE_REG_FAILED)
    {
		   //    Led1_Set(LED_OFF);
			     //nbiot_device_close(dev,10);
           //nbiot_register_start(dev,100,buffer,buffer_len);         
    }
}
