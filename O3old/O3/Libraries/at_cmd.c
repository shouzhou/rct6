#include "bsp.h"
sendmsg callback = NULL;
volatile char *flag_ok = NULL;
struct ringbuf *result_ptr = NULL;
void register_cmd_handler(sendmsg func, void *result_buf, volatile char *flag)
{
    callback = func;
    result_ptr = (struct ringbuf *)result_buf;
    flag_ok = flag;
}
char SendCmd(char *cmd, uint8_t *result, uint16_t timeout, uint8_t retry, uint16_t waittime)
{
    //	printf("send cmd:%s",cmd);
    char *msg_p = NULL;
    //char *msg_clk = NULL;
    time_t nowtime = 0, newtime = 0, sum = 0;
    uint8_t retry_num = 0, retryflag = 0;
    if (callback == NULL || result_ptr == NULL || flag_ok == NULL)
        return 2;
    *flag_ok = 0;
    ringbuf_clear(result_ptr); //清除之前可能残留的信息
    printf("cmd:%s\r\n", cmd);
    callback((uint8_t *)cmd, strlen((const char *)cmd));
    nowtime = nbiot_time();
    while (1)
    {
        if (sum > waittime)
        {
            if (++retry_num > retry)
            {
                g_FailTime++;
                return 0;
                
            }
            retryflag = 1;
        }
        if (*flag_ok == 1)
        {
            printf("cmd_rsp:%s", result_ptr->data);
            *flag_ok = 0;
            /////////-----------------更新时间--------begin---------------///
//            msg_clk = strstr(result_ptr->data, "+CCLK");
//            if (msg_clk != NULL)
//            {
//                UpdateTime(msg_clk);
//                ringbuf_clear(result_ptr);
//                break;
//            }
             /////////-----------------更新时间--------end---------------///
            
            
            msg_p = strstr(result_ptr->data, result);
            if (msg_p != NULL)
            {
                ringbuf_clear(result_ptr);
                break;
            }
            else
            {

                msg_p = strstr(result_ptr->data, "ERROR");
                ringbuf_clear(result_ptr);
                if (msg_p != NULL)
                    return 2;
            }
        }
        else
        {
            newtime = nbiot_time();
            sum = newtime - nowtime;
        }
        if (retryflag == 1)
        {
            retryflag = 0;
            sum = 0;
            nowtime = nbiot_time();
            if (retry_num > 0 && retry_num < retry + 1)
            {

                printf("retry cmd:%s", cmd);
                callback((uint8_t *)cmd, strlen((const char *)cmd));
            }
        }
    }
    bsp_DelayMS(timeout);
    return 1;
}
char SendCmd1(char *cmd, uint8_t *result, uint16_t timeout, uint8_t retry, uint16_t waittime)
{
    //	printf("send cmd:%s",cmd);
    char *msg_p = NULL;
    char *msg_clk = NULL;
    time_t nowtime = 0, newtime = 0, sum = 0;
    uint8_t retry_num = 0, retryflag = 0;
    if (callback == NULL || result_ptr == NULL || flag_ok == NULL)
        return 2;
    *flag_ok = 0;
    ringbuf_clear(result_ptr); //清除之前可能残留的信息
    printf("cmd:%s\r\n", cmd);
    callback((uint8_t *)cmd, strlen((const char *)cmd));
    nowtime = nbiot_time();
    while (1)
    {
        if (sum > waittime)
        {
            if (++retry_num > retry)
                return 0;
            retryflag = 1;
        }
        if (*flag_ok == 1)
        {
            printf("cmd_rsp:%s", result_ptr->data);
            *flag_ok = 0;
            /////////-----------------更新时间--------begin---------------///
            msg_clk = strstr(result_ptr->data, "+CCLK");
            if (msg_clk != NULL)
            {
                UpdateTime(msg_clk);
                ringbuf_clear(result_ptr);
                break;
            }
             /////////-----------------更新时间--------end---------------///
            
            
//            msg_p = strstr(result_ptr->data, result);
//            if (msg_p != NULL)
//            {
//                ringbuf_clear(result_ptr);
//                break;
//            }
//            else
//            {

//                msg_p = strstr(result_ptr->data, "ERROR");
//                ringbuf_clear(result_ptr);
//                if (msg_p != NULL)
//                    return 2;
//            }
        }
        else
        {
            newtime = nbiot_time();
            sum = newtime - nowtime;
        }
        if (retryflag == 1)
        {
            retryflag = 0;
            sum = 0;
            nowtime = nbiot_time();
            if (retry_num > 0 && retry_num < retry + 1)
            {

                printf("retry cmd:%s", cmd);
                callback((uint8_t *)cmd, strlen((const char *)cmd));
            }
        }
    }
    bsp_DelayMS(10);
    return 1;
}
void SentData(char *cmd, uint8_t *result, uint16_t timeout)
{
   // char *msg_p = NULL;
    time_t nowtime = 0, newtime = 0, sum = 0;
    if (callback == NULL || result_ptr == NULL || flag_ok == NULL)
        return;
    *flag_ok = 0;
    ringbuf_clear(result_ptr); //清除之前可能残留的信息
    printf("Data:%s\r\n", cmd);
    callback((uint8_t *)cmd, strlen((const char *)cmd));
    nowtime = nbiot_time();

    while (1)
    {
        if (sum > 2)
            break;
        if (*flag_ok == 1)
        {
            *flag_ok = 0;
            printf("data_rsp:%s\r\n", result_ptr->data);

            break;
        }
        else
        {

            newtime = nbiot_time();
            sum = newtime - nowtime;
        }
    }
    bsp_DelayMS(2);
}

void UpdateTime( char *rev)
{
    uint8_t m_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t *p= (uint8_t *)rev;
    uint16_t _updateyear;
    uint8_t _updatemonth,_updatedate,_updatehour,_updatemin,_updatesec;
    _updateyear= (p[6]-0x30)*10+(p[7]-0x30) +2000;
    if(Is_Leap_Year(_updateyear))
    {
        m_table[1] = 29;
    }
    _updatemonth= (p[9]-0x30)*10+(p[10]-0x30);
    _updatedate= (p[12]-0x30)*10+(p[13]-0x30);
    
    
    _updatehour= (p[15]-0x30)*10+(p[16]-0x30);
    
    _updatemin= (p[18]-0x30)*10+(p[19]-0x30);
    _updatesec= (p[21]-0x30)*10+(p[22]-0x30);
    if(_updatehour>=16) //gmt时间转换为北京时间
    {
        _updatehour= _updatehour -16;
        if(_updatedate == m_table[_updatemonth-1]) //到月尾了 需要day =1 mon+1
        {
            _updatedate = 1;
            if(_updatemonth ==12)
            {
                _updatemonth = 1;
                _updateyear = _updateyear +1;
            }
            else
            {
                _updatemonth= _updatemonth + 1;
            }
        }
        else
        {
            _updatedate = _updatedate + 1;
        }
    }
    else
    {
        _updatehour= _updatehour + 8;
    }
    RTC_WriteClock(_updateyear,_updatemonth,_updatedate,_updatehour,_updatemin,_updatesec);
    
}
