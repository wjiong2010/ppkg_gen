#include <stdio.h>
#include <windows.h>

#define COM_RD_BUF      4096
#define COM_WR_BUF      4096

static void com_timeout_set(HANDLE hCom)
{
    COMMTIMEOUTS TimeOuts;

    memset(&TimeOuts, 0, sizeof(COMMTIMEOUTS));
    
    //设定读超时
    TimeOuts.ReadIntervalTimeout = 20;//读间隔超时 MAXDWORD
    TimeOuts.ReadTotalTimeoutMultiplier = 1;//读时间系数
    TimeOuts.ReadTotalTimeoutConstant = 100;//读时间常量
    //设定写超时
    TimeOuts.WriteTotalTimeoutMultiplier = 1;//写时间系数
    TimeOuts.WriteTotalTimeoutConstant = 1;//写时间常量
    SetCommTimeouts(hCom, &TimeOuts); //设置超时
}

static int com_config(HANDLE hCom, int bd_rate, int byte_size)
{
    DCB dcb = {0};
    char com_cfg_str[32] = {0};
    
    if (!GetCommState(hCom, &dcb))
    {
        printf("GetCommState Failed, close com");
        CloseHandle(hCom);
        return -1;
    }

    snprintf(com_cfg_str, 32, "%d,n,%d,1", bd_rate, byte_size);
    printf("com_cfg_str:%s \n", com_cfg_str);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB(com_cfg_str,&dcb))//填充DCB的数据传输率、奇偶校验类型、数据位、停止位
    {
        printf("GetCommState Failed, close com");
        CloseHandle(hCom);
        return -1;

    }
    
    if(SetCommState(hCom, &dcb))
    {
        printf("SetCommState success");
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

    printf("create_com, name:%s, rate:%d, byte_size:%d \n",
            p_com_name, bd_rate, byte_size);
     
    hCom = CreateFile(TEXT(p_com_name),
            GENERIC_READ | GENERIC_WRITE,   //允许读写
            0,                              //指定共享属性，由于串口不能共享，所以该参数必须为0
            NULL,
            OPEN_EXISTING,                  //打开而不是创建
            FILE_ATTRIBUTE_NORMAL,          //属性描述，FILE_FLAG_OVERLAPPED表示使用异步I/O， FILE_ATTRIBUTE_NORMAL 表示同步I/O操作
            NULL);

    printf("create_com hCom:%p\n", hCom);

    if (hCom == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile, failed!\n");
        return INVALID_HANDLE_VALUE;
    }

    if (!SetupComm(hCom, COM_WR_BUF, COM_RD_BUF)) //输入缓冲区和输出缓冲区的大小都是1024
    {
        printf("SetupComm, failed!\n");
        return INVALID_HANDLE_VALUE;
    }
    
    com_timeout_set(hCom);
    
    if (-1 == com_config(hCom, bd_rate, byte_size))
    {
        printf("com_config, failed!\n");
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
        PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);
        printf("com_write, PurgeComm\n");
    }

    if (0 == WriteFile(hCom, pbuf, len, &wCount, NULL))
    {
        wCount = 0;
    }
    
    printf("wCount:%d, len:%d\n", (int)wCount, len);
    
    return wCount;
}

DWORD com_read(HANDLE hCom, char* read_buf, int buf_size)
{
    DWORD dwError;
    DWORD wCount = 0;
    unsigned char bReadStat;
    char buff[128] = {0};
    int i = 0;
 
    if (ClearCommError(hCom, &dwError, NULL))
    {
        PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
        printf("com_read, PurgeComm\n");
    }

    while(1)
    {
        bReadStat = ReadFile(hCom, (char*)&buff[i], 128, &wCount, NULL);

        i += wCount;
        printf("bReadStat:%d\n", bReadStat);

        if (i >= 128)
        {
            printf("ReadFile buff full! \n");
            wCount = i;
            break;
        }
            
        if (!bReadStat)
        {
            printf("ReadFile failed! \n");
            wCount = 0;
            break;
        }

        if (bReadStat && !wCount)
        {
            printf("ReadFile completed! \n");
            wCount = i;
            break;
        }
    }

    memcpy(read_buf, buff, wCount);
    
    return wCount;
}


