/************************************************************************************
* FileName: ppkg_main.c
*
* Author: John.Wang
*
* Date: 2019-02-16
*
* Descripiton: generator of a param package auto set
*************************************************************************************/
/************************************************************************************
* Include
*************************************************************************************/
#include "ppkg_com.h"
#include "ppkg_main.h"
#include "debug.h"
#include "ppkg_generator.h"

/************************************************************************************
* Define and Declaration
*************************************************************************************/
static char read_buff[5120];

/************************************************************************************
* Process
*************************************************************************************/
/************************************************************************************
* Function: main
* Author @ Date: John.Wang@20200216
* Input:
* Return:
* Description: main function of the generator of a param package auto set
*************************************************************************************/
int main(void)
{
    #if 0
    HANDLE com_handle = INVALID_HANDLE_VALUE;
    static int init_flag = 0;
    int real_len;
    
    com_handle = create_com("com3", 115200, 8);
    if (INVALID_HANDLE_VALUE == com_handle)
    {
        DBG_TRACE("open com failed!");
        return 0;
    }    

    while(1)
    {
        if (!init_flag)
        {
            int cmd_len = strlen("AT+CSUB\r\n");
            real_len = com_write(com_handle, "AT+CSUB\r\n", cmd_len);
            DBG_TRACE("com_write real_len:%d, len:%d", real_len, cmd_len);
            init_flag = 1;
        }
        //Sleep(100);

        real_len = com_read(com_handle, read_buff, 5120);
        DBG_TRACE("->%d, %s", real_len, read_buff);

        memset(read_buff, 0, 5210);
    }

    destroy_com(com_handle);
    #endif
    ppkg_gen();
    return 0;
}