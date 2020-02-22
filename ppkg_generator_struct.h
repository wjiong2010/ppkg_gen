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
#define MAX_LEN_CMD_TYPE    3

#define T_FILE_READ_ONLY		"r"
#define T_FILE_WRITE_ONLY		"w"
#define T_FILE_WRITE_ADD		"a+"
#define B_FILE_READ_ONLY		"rb"

#define IS_EXIST        F_OK
#define READ_EN         R_OK
#define WRITE_EN        W_OK
#define EXEC_EN         X_OK

#define MAX_MEM_BLOCK_LEN	52100	/* 50K bytes block */
#define MAX_TEMP_BLOCK_LEN  10240   /* 10K bytes block */

/* A command frame sample:
 * <Command Type="GTASC" BrakeSpeedThreshold="60" DeltaSpeedThreshold="8" DeltaHeadingThreshold="5"/>
 */
#define CMD_FRAME_HEAD_LEN  14      /* <Command Type= */
#define CMD_PREFIX_LEN      2       /* GT */
#define CMD_ATGT_HEAD_LEN   5
/*
AtCmd[1]=
MetaResult[1]=OK
*/
#define MAX_ATFILE_ITEM_FMT_LEN   64

#define LINE_CMD_FLAG       "<Command"
#define LINE_VERSION_FLAG   "<Commands"
#define LINE_ATGT_CMD       "AT+GT"

#define VERSION_TITLE_STR   "FirmwareSubVersion="
#define AT_FILE_HEAD_STR    "[ConfirMeta]\r\n\r\n"
#define AT_FILE_TAIL_STR    "THE END"
#define AT_CFG_FILE_NAME    "ATFILE.ini"

/************************************************************************************
* Enums
*************************************************************************************/
typedef enum {
    CFG_LINE,
    CMD_LINE,
    VER_LINE,
    UNKOWN_LINE
} line_type_enum;

/************************************************************************************
* Structs
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
    char    cmd_type[MAX_LEN_CMD_TYPE + 1];
    int     cmd_len;
    char    cmd_str[1];
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

typedef struct 
{
    FILE *temp_fp;
    int   cmd_cnt;
} ppkg_gen_context;

#endif /* __PPKG_GENERATOR_STRUCT_H__ */


