/************************************************************************************
* FileName: version.c
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
#define version_str PPKG_VER_STR
/************************************************************************************
* Process
*************************************************************************************/
/************************************************************************************
* Function: print_ver
* Author @ Date: John.Wang @ 20200901
* Input:
* Return:
* Description:
*************************************************************************************/
void print_ver(void)
{
    printf("PPKG_VER:%s", version_str);
}

