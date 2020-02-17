/************************************************************************************
* FileName: ppkg_generator_struct.h
*
* Author: John.Wang
*
* Date: 2020-02-16
*
* Descripiton: A generator for param package auto set, head file of pre-parser
*************************************************************************************/
#ifndef __PPKG_GENERATOR_STRUCT_H__
#define __PPKG_GENERATOR_STRUCT_H__
/************************************************************************************
* Include
*************************************************************************************/
/************************************************************************************
* Define and Declaration
*************************************************************************************/
#define MAX_LEN_PATH        128
#define MAX_LEN_FW_VER      64
#define MAX_CMD_LIST_SIZE   1000

#define T_FILE_READ_ONLY		"r"
#define T_FILE_WRITE_ONLY		"w"
#define T_FILE_WRITE_ADD		"a+"
#define B_FILE_READ_ONLY		"rb"

#define MAX_MEM_BLOCK_LEN	52100	/* 50K bytes block */
#define MAX_TEMP_BLOCK_LEN  10240   /* 10K bytes block */

/************************************************************************************
* Extern Function 
*************************************************************************************/
typedef struct {
    char fm_version[MAX_LEN_FW_VER];
    char path_def_cfg[MAX_LEN_PATH];
    char path_cust_cfg[MAX_LEN_PATH];
    char path_cust_ini[MAX_LEN_PATH];
} cfg_info_struct;

typedef struct _node {
    struct _node  *pre;
    struct _node  *next;
    int     cmd_type;
    char*   cmd_str;
} cmd_node_struct;

typedef struct {
    cmd_node_struct *qhead;
    cmd_node_struct *qtail;
    int len;
} cfg_list_queue;

typedef struct {
    cfg_list_queue def_cfg;
    cfg_list_queue cust_cfg;
    cfg_list_queue cust_ini;
    cfg_list_queue diff_cfg;
} cmd_list_struct;

typedef struct 
{
	int rd_pos;
	int wr_pos;
	int valid_bytes;
	char *data;
} circal_buffer;


#endif /* __PPKG_GENERATOR_STRUCT_H__ */


