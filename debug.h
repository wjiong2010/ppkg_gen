/************************************************************************************
* FileName: debug.h
*
* Author: John.Wang
*
* Date: 2019-03-15
*
* Descripiton: Log Line Parser 
*************************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__
/************************************************************************************
* Include
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
/************************************************************************************
* Extern Function 
*************************************************************************************/
typedef unsigned char bool;

/************************************************************************************
* Extern Function 
*************************************************************************************/
extern int debug_print_log(char *fmt, ...);

/************************************************************************************
* Macros
*************************************************************************************/
#define MAX_TRACE_LOG_LEN  1024
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif



#if 1 
#define DBG_TRACE(format, ...)\
	do\
	{\
		debug_print_log("%s %s[%10s][%4d]"format"\n", __DATE__, __TIME__, __func__, __LINE__, ##__VA_ARGS__);\
	}\
	while(0);
#else	/* __DEBUG_SUPPORT__*/
#define DBG_TRACE(format, ...)\
	do\
	{\
	}\
	while(0);

#endif /* __DEBUG_SUPPORT__*/


#endif /* __DEBUG_H__ */

