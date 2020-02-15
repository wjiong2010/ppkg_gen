#include <windows.h>

extern HANDLE create_com(char *p_com_name, int bd_rate, int byte_size);
extern void destroy_com(HANDLE hCom);
extern DWORD com_write(HANDLE hCom, char* pbuf, int len);
extern DWORD com_read(HANDLE hCom, char* pbuf, int buf_size);

