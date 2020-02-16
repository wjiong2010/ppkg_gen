#include <stdio.h>
#include <windows.h>

#include "debug.h"

#define COM_RD_BUF      4096
#define COM_WR_BUF      4096

static void com_timeout_set(HANDLE hCom)
{
    COMMTIMEOUTS TimeOuts;

    memset(&TimeOuts, 0, sizeof(COMMTIMEOUTS));
    
    /* Set read timeout
     * read interval timeout,means the time interval between to byte read. max value is MAXDWORD 
     */
    TimeOuts.ReadIntervalTimeout = 20;
    /* Multiply by total number of bytes ready to read. Unit is ms */
    TimeOuts.ReadTotalTimeoutMultiplier = 1;
    /* A constant value used to calculate total read timeout
     * formula is total_read_timeout =  ReadTotalTimeoutMultiplier x read_bytes + ReadTotalTimeoutConstant
     */ 
    TimeOuts.ReadTotalTimeoutConstant = 100;
    /* Set write timeout
     */
    TimeOuts.WriteTotalTimeoutMultiplier = 1;   //写时间系数
    TimeOuts.WriteTotalTimeoutConstant = 1;     //写时间常量
    SetCommTimeouts(hCom, &TimeOuts);           //设置超时
}

static int com_config(HANDLE hCom, int bd_rate, int byte_size)
{
    DCB dcb = {0};
    char com_cfg_str[32] = {0};
    
    if (!GetCommState(hCom, &dcb))
    {
        DBG_TRACE("GetCommState Failed, close com");
        CloseHandle(hCom);
        return -1;
    }

    /* Baudrate, Parity:None, Byte size, Stop bit: 1 */
    snprintf(com_cfg_str, 32, "%d,n,%d,1", bd_rate, byte_size);
    DBG_TRACE("com_cfg_str:%s", com_cfg_str);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB(com_cfg_str,&dcb))
    {
        DBG_TRACE("BuildCommDCB Failed, close com");
        CloseHandle(hCom);
        return -1;
    }
    
    if(!SetCommState(hCom, &dcb))
    {
        DBG_TRACE("SetCommState Failed, close com");
        CloseHandle(hCom);
        return -1;
    }
    #if 0
    dcb.BaudRate = bd_rate;     //baudrate 
    dcb.ByteSize = byte_size;   //byte size
    dcb.Parity = NOPARITY;      //无奇偶校验位
    dcb.StopBits = ONESTOPBIT;  //一个停止位
    SetCommState(hCom, &dcb);
    #endif
    return 0;
}

void destroy_com(HANDLE hCom)
{
    CloseHandle(hCom);
}

HANDLE create_com(char *p_com_name, int bd_rate, int byte_size)
{
    HANDLE hCom = INVALID_HANDLE_VALUE;

    DBG_TRACE("create_com, name:%s, rate:%d, byte_size:%d",
            p_com_name, bd_rate, byte_size);
     
    hCom = CreateFile(TEXT(p_com_name),
            GENERIC_READ | GENERIC_WRITE,   /* Read and Write enable */
            0,                              /* Sharing Attribute, we don't share serial port so must set to 0 */
            NULL,
            OPEN_EXISTING,                  /* Open existe serial port, we can't create a new serial port */
            FILE_ATTRIBUTE_NORMAL,          /* FILE_FLAG_OVERLAPPED means asynchronous IO 
                                             * FILE_ATTRIBUTE_NORMAL means synchronized-IO. we used here
                                             */
            NULL);

    DBG_TRACE("create_com hCom:%p", hCom);

    if (hCom == INVALID_HANDLE_VALUE)
    {
        DBG_TRACE("CreateFile, failed!");
        return INVALID_HANDLE_VALUE;
    }

    /* Set read & write buffer size */
    if (!SetupComm(hCom, COM_WR_BUF, COM_RD_BUF))
    {
        DBG_TRACE("SetupComm, failed!");
        return INVALID_HANDLE_VALUE;
    }
    
    com_timeout_set(hCom);
    
    if (-1 == com_config(hCom, bd_rate, byte_size))
    {
        DBG_TRACE("com_config, failed!");
        return INVALID_HANDLE_VALUE;
    }
    
    return hCom;
}

DWORD com_write(HANDLE hCom, char* pbuf, int len)
{
    DWORD dwError;
    DWORD wCount = 0;
    
    if (ClearCommError(hCom, &dwError, NULL))
    {
        /* Stop send and clear send buffer. */
        PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);
        DBG_TRACE("com_write, PurgeComm");
    }

    if (0 == WriteFile(hCom, pbuf, len, &wCount, NULL))
    {
        wCount = 0;
    }
    
    DBG_TRACE("com_write real_len:%d, len:%d, buf:%s", (int)wCount, len, pbuf);
    
    return wCount;
}

DWORD com_read(HANDLE hCom, char* read_buf, int buf_size)
{
    #define MAX_BUF_SIZE 128

    DWORD dwError;
    DWORD wCount = 0;
    unsigned char bReadStat;
    char buff[MAX_BUF_SIZE] = {0};
    int i = 0;

    if (NULL == read_buf || 0 == buf_size)
    {
        DBG_TRACE("com_read, NULL buffer, return!");
        return wCount;
    }
 
    if (ClearCommError(hCom, &dwError, NULL))
    {
        PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
        DBG_TRACE("com_read, PurgeComm");
    }

    while(1)
    {
        bReadStat = ReadFile(hCom, (char*)&buff[i], MAX_BUF_SIZE, &wCount, NULL);

        i += wCount;
        DBG_TRACE("bReadStat:%d", bReadStat);

        if (i >= MAX_BUF_SIZE)
        {
            DBG_TRACE("ReadFile buff full!");
            wCount = i;
            break;
        }
            
        if (!bReadStat)
        {
            DBG_TRACE("ReadFile failed!");
            wCount = 0;
            break;
        }

        if (bReadStat && !wCount)
        {
            DBG_TRACE("ReadFile completed!");
            wCount = i;
            break;
        }
    }

    memcpy(read_buf, buff, wCount);
    
    return wCount;
}


