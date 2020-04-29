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
* types
*************************************************************************************/
typedef unsigned char bool;

/************************************************************************************
* Macros & enums
*************************************************************************************/
#define MAX_TRACE_LOG_LEN  1024
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


typedef enum
{
	INF_NOTICE = 0,
	DBG_LOG,

	DEBUG_NUM
} debug_type_enums;
/************************************************************************************
* Extern Function
*************************************************************************************/
extern int debug_print_log(char *fmt, ...);
/************************************************************************************
* Function: debug_set_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
extern void debug_set_print_info_mask(debug_type_enums dbg_type);

/************************************************************************************
* Function: debug_clear_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
extern void debug_clear_print_info_mask(debug_type_enums dbg_type);

/************************************************************************************
* Function: debug_reset_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
extern void debug_reset_print_info_mask(void);

/************************************************************************************
* Function: debug_get_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
extern unsigned int debug_get_print_info_mask(void);

/************************************************************************************
* Function: debug_check_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
extern bool debug_check_print_info_mask(debug_type_enums dbg_type);


/************************************************************************************
* Main Macro
*************************************************************************************/
#define DBG_TRACE(_tYPE_, _fMT_, ...)\
			do\
			{\
				if (INF_NOTICE == _tYPE_)\
				{\
					debug_print_log(_fMT_, ##__VA_ARGS__);\
				}\
				else if (debug_check_print_info_mask(_tYPE_))\
				{\
					debug_print_log("%s %s[%10s][%4d]"_fMT_"\n", __DATE__, __TIME__, __func__, __LINE__, ##__VA_ARGS__);\
				}\
				else \
				{\
					;\
				}\
			}\
			while(0);

#endif /* __DEBUG_H__ */

