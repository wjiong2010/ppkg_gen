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

/************************************************************************************
* Process
*************************************************************************************/
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

	strftime(pbuff,buff_size,"%Y%m%e_%H%M%S", info);//以年月日_时分秒的形式表示当前时间

	return strlen(pbuff);
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
	int len;
	int temp_len;
	char *buff_p = g_output_log_buf;
	va_list list;

	memset(buff_p, 0, sizeof(g_output_log_buf));

	/* get local time */
	//len = debug_get_time(buff_p, MAX_TRACE_LOG_LEN);
	len = snprintf((char *)(buff_p +len), MAX_TRACE_LOG_LEN - len, "%s %s", __DATE__, __TIME__);

	/* package function name */
	temp_len = strlen(func_name);
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



