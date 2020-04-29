/************************************************************************************
* FileName: debug.c
*
* Author: John.Wang
*
* Date: 2019-03-15
*
* Descripiton: print debug info to standard output
*************************************************************************************/
/************************************************************************************
* Include
*************************************************************************************/
#include <time.h>
#include "debug.h"
/************************************************************************************
* Define and Declaration
*************************************************************************************/
static char g_output_log_buf[MAX_TRACE_LOG_LEN] = {0};
static unsigned int debug_info_mask = 0;
/************************************************************************************
* Process
*************************************************************************************/
#if 0
static int debug_get_time(char *pbuff, int buff_size)
{
	time_t rawtime;
	struct tm *info;

	if (NULL == pbuff)
	{
		return 0;
	}

	time( &rawtime );
	info = localtime( &rawtime );

	strftime(pbuff,buff_size,"%Y%m%e_%H%M%S", info);

	return strlen(pbuff);
}
#endif

/************************************************************************************
* Function: debug_set_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
void debug_set_print_info_mask(debug_type_enums dbg_type)
{
	if (dbg_type < DEBUG_NUM)
	{
		debug_info_mask |= (0x01 << dbg_type);
	}
}

/************************************************************************************
* Function: debug_clear_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
void debug_clear_print_info_mask(debug_type_enums dbg_type)
{
	if (dbg_type < DEBUG_NUM)
	{
		debug_info_mask &= ~(0x01 << dbg_type);
	}
}

/************************************************************************************
* Function: debug_check_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
bool debug_check_print_info_mask(debug_type_enums dbg_type)
{
	return (bool)(debug_info_mask & (0x01 << dbg_type));
}

/************************************************************************************
* Function: debug_reset_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
void debug_reset_print_info_mask(void)
{
	debug_info_mask = 0;
}

/************************************************************************************
* Function: debug_get_print_info_mask
* Author @ Date: John.Wang @ 20200429
* Input:
* Return:
* Description:
*************************************************************************************/
unsigned int debug_get_print_info_mask(void)
{
	return debug_info_mask;
}


/************************************************************************************
* Function: debug_print_log_full
* Author @ Date:
* Input:
* Return:
* Description:
*************************************************************************************/
int debug_print_log_full(int log_line, char *func_name, char *fmt, ...)
{
	int len = 0;
	//int temp_len;
	char *buff_p = g_output_log_buf;
	va_list list;

	memset(buff_p, 0, sizeof(g_output_log_buf));

	/* get local time */
	//len = debug_get_time(buff_p, MAX_TRACE_LOG_LEN);
	len = snprintf((char *)(buff_p +len), MAX_TRACE_LOG_LEN - len, "%s %s", __DATE__, __TIME__);

	/* package function name */
	//temp_len = strlen(func_name);
	len += snprintf((char *)(buff_p +len), MAX_TRACE_LOG_LEN - len, "[%s]", func_name);

	/* trace log body */
	va_start(list,fmt);
	len += vsnprintf((char*)(buff_p + len), MAX_TRACE_LOG_LEN - len, fmt, list);
	va_end(list);

	/* line number */
	len += snprintf((char *)(buff_p +len), MAX_TRACE_LOG_LEN - len, "L-%d", log_line);

	return len;
}

/************************************************************************************
* Function: debug_print_log
* Author @ Date:
* Input:
* Return:
* Description:
*************************************************************************************/
int debug_print_log(char *fmt, ...)
{
	int len;
	va_list list;

	/* trace log body */
	va_start(list,fmt);
	len = vprintf(fmt, list);
	va_end(list);

	return len;
}



