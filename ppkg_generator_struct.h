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
#include <unistd.h>
#include <windows.h>

/************************************************************************************
* Define and Declaration
*************************************************************************************/
#define HFILE           FILE
#define HCOM            HANDLE

#define MAX_LEN_PATH        128
#define MAX_LEN_FW_VER      64
#define MAX_CMD_LIST_SIZE   1000
#define MAX_LEN_CMD_TYPE    3
#define MAX_TEMP_BUFF_LEN   512
#define MAX_QUERY_CMD_LEN   32
#define MAX_COMMAND_NUM     0x0fffffff

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
#define MAX_ID_LIST_LEN     1024

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
#define LINE_ATCSUB_CMD     "AT+CSUB"


#define VERSION_TITLE_STR   "FirmwareSubVersion="
#define AT_FILE_HEAD_STR    "[ConfirMeta]\r\n\r\n"
#define AT_FILE_TAIL_STR    "THE END"
#define AT_CFG_FILE_NAME    "ATFILE.ini"
#define AT_CFM_FILE_NAME    "CFMFILE.ini"
#define COMMA_SEPARATOR     ','
/************************************************************************************
* Enums
*************************************************************************************/
typedef enum {
    CFG_LINE,
    CMD_LINE,
    VER_LINE,
    UNKOWN_LINE
} line_type_enum;

typedef enum {
    GEN_SUCCESS = 0,
    GEN_CONFIG_FAIL = -1,
    GEN_CMD_LIST_FAIL = -2,
    GEN_ATFILE = -3,
    GEN_COMM = -4,
    GEN_PRE_CHECK = -5,
    GEN_CONFIRM = -6
} gen_result_enum;


/************************************************************************************
* Structs
*************************************************************************************/
typedef struct {
    char fm_version[MAX_LEN_FW_VER];
    char path_def_cfg[MAX_LEN_PATH];
    char path_cust_cfg[MAX_LEN_PATH];
    char path_cust_ini[MAX_LEN_PATH];
    char password[MAX_LEN_PATH];
} cfg_info_struct;

typedef struct _node {
    struct _node  *pre;
    struct _node  *next;
} __node_struct;
typedef __node_struct link_type;

typedef struct {
    link_type   qlink;
    int         len;
} __queue_struct;
typedef __queue_struct queue_type;

typedef struct {
    int         diff_id_num;
    int         *diff_id_list;
} cmd_diff_data;

typedef struct {
    link_type   qlink;
    char        cmd_type[MAX_LEN_CMD_TYPE + 1];
    bool        is_multi_cmd;
    cmd_diff_data diff_data;
} diff_cmd_node;

typedef struct {
    link_type   qlink;
    char        cmd_type[MAX_LEN_CMD_TYPE + 1];
    bool        is_multi_cmd;
    int         id;             /* Only for multi-command */
    int         cmd_len;
    char        cmd_str[1];
} cmd_node_struct;

typedef struct {
    char    *cmd_name;
    bool    ignored;
    char    *first_param;
    int     id_pos;             /* for multi-command, means how many ',' before <id>.
                                 * for single-command, always be 0
                                 */
    char    *key_paras;
} cmd_attribute_struct;

typedef struct {
	int rd_pos;
	int wr_pos;
	int valid_bytes;
	char *data;
} circal_buffer;

typedef struct {
    HFILE       *temp_fp;
    int         cmd_cnt;
    char        pre_cmd_type[MAX_LEN_CMD_TYPE + 1];
    queue_type  default_cfg;
    queue_type  custom_cfg;
    queue_type  custom_ini;
    queue_type  differ_cfg;
    queue_type  at_queue;
    cmd_node_struct *temp_node_p;
    cmd_node_struct *first_cfg_p;
    bool        pwd_changed;
    HCOM        com_hdlr;
    circal_buffer cir_buff;
} ppkg_gen_context;

#endif /* __PPKG_GENERATOR_STRUCT_H__ */


