/************************************************************************************
* FileName: ppkg_com.h
*
* Author: John.Wang
*
* Date: 2020-02-16
*
* Descripiton: A generator for param package auto set, head file of serial port
*************************************************************************************/
#ifndef __PPKG_COM_H__
#define __PPKG_COM_H__
/************************************************************************************
* Include
*************************************************************************************/
#include <windows.h>
/************************************************************************************
* Extern Function 
*************************************************************************************/
extern HANDLE create_com(char *p_com_name, int bd_rate, int byte_size);
extern void destroy_com(HANDLE hCom);
extern DWORD com_write(HANDLE hCom, char* pbuf, int len);
extern DWORD com_read(HANDLE hCom, char* pbuf, int buf_size);

/************************************************************************************
* Macros
*************************************************************************************/

#endif /* __PPKG_COM_H__ */

