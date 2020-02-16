#include "ppkg_com.h"
#include "ppkg_main.h"
#include "debug.h"

char read_buff[5120];

int main(void)
{
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
    
    return 0;
}