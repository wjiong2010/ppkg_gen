/************************************************************************************
* FileName: ppkg_generator.c
*
* Author: John.Wang
*
* Date: 2020-02-16
*
* Descripiton: generator param auto set package
*************************************************************************************/
/************************************************************************************
* Include
*************************************************************************************/
#include "debug.h"
#include "ppkg_generator_struct.h"
#include "ppkg_com.h"

/************************************************************************************
* Define and Declaration
*************************************************************************************/
static char memory_block[MAX_MEM_BLOCK_LEN] = {0};
static char temp_buff[MAX_TEMP_BLOCK_LEN] = {0};
static char read_buff[MAX_TEMP_BLOCK_LEN] = {0};
static int  command_id_list[MAX_ID_LIST_LEN] = {0};

static cmd_attribute_struct command_attr_table[] =
{
    /* cmd_name, ignored, first_param, id_pos, key parameters */
    {  "GEO",    FALSE,   "GEOID",      1,      "Mode" },
    {  "PEO",    FALSE,   "GEOID",      1,      "Mode" },
    {  "CLT",    FALSE,   "GroupID",    1,      NULL },
    {  "FFC",    FALSE,   "Priority",   1,      NULL },
    {  "IOB",    FALSE,   "IOBID",      1,      NULL },
    {  "CMD",    FALSE,   "Mode",       2,      "Mode,CommandString" },
    {  "UDF",    FALSE,   "Mode",       2,      "Mode,InputIdMask" },
    {  "TMP",    FALSE,   "Mode",       1,      "Mode,SensorID" },
    {  "FSC",    FALSE,   "TableID",    2,      "SensorType,Enable" },
    {  "RTO",    TRUE,    NULL,         0,      NULL  },
    {  "UPD",    TRUE,    NULL,         0,      NULL  },
    {  "FVR",    TRUE,    NULL,         0,      NULL  },
    {  "QSS",    TRUE,    NULL,         0,      NULL  },
    {  NULL,     TRUE,    NULL,         0,      NULL }
};

static cfg_info_struct cfg_info;
static ppkg_gen_context ppkg_context_st;
static ppkg_gen_context *ppkg_cntx_p = &ppkg_context_st;

/************************************************************************************
* Types
*************************************************************************************/
typedef int (*cb_func)(queue_type *cmp_q, link_type *cust_node, link_type *def_node);

/************************************************************************************
* Process
*************************************************************************************/
/************************************************************************************
* Function: q_init
* Author @ Date: John.Wang@20200308
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void q_init(queue_type *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE(DBG_LOG,"return q_init NULL");
        return;
    }

    queue_p->qlink.pre = queue_p->qlink.next = (link_type*)(&queue_p->qlink);
    queue_p->len = 0;
}

/************************************************************************************
* Function: q_destroy
* Author @ Date: John.Wang@20200411
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void q_destroy(queue_type *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE(DBG_LOG,"return q_destroy NULL");
        return;
    }

    queue_p->qlink.pre = queue_p->qlink.next = NULL;
    queue_p->len = 0;
}

/************************************************************************************
* Function: q_put
* Author @ Date: John.Wang@20200308
* Input:
* Return: 
* Description: 
*************************************************************************************/
int q_put(queue_type *queue_p, link_type *node)
{
    link_type *head = NULL;
    
    if (NULL == queue_p || NULL == node)
    {
        DBG_TRACE(DBG_LOG,"return q_put NULL");
        return -1;
    }

    head = (link_type*)(&queue_p->qlink);

    node->next = head;
    node->pre = head->pre;
    head->pre->next = node;
    head->pre = node;

    queue_p->len++;
    
    return queue_p->len;
}

/************************************************************************************
* Function: q_check
* Author @ Date: John.Wang@20200308
* Input:
* Return: 
* Description: checkout the head node of queue
*************************************************************************************/
link_type *q_check(queue_type *queue_p)
{
    link_type *head = NULL;
    link_type *node = NULL;
    
    if (NULL == queue_p || 0 == queue_p->len)
    {
        DBG_TRACE(DBG_LOG,"return q_check NULL");
        return NULL;
    }
    
    head = (link_type*)(&queue_p->qlink);

    node = head->next;
    head->next = node->next;
    node->next->pre = node->pre;

    queue_p->len--;
    
    return node;
}

/************************************************************************************
* Function: q_get
* Author @ Date: John.Wang@20200409
* Input:
* Return: 
* Description: get the node next the head but do not checkout
*************************************************************************************/
link_type *q_get(queue_type *queue_p)
{
    if (NULL == queue_p || 0 == queue_p->len)
    {
        DBG_TRACE(DBG_LOG,"return q_get NULL");
        return NULL;
    }

    return queue_p->qlink.next;
}

/************************************************************************************
* Function: q_next
* Author @ Date: John.Wang@20200409
* Input:
* Return: 
* Description: get the node next the link_p but do not checkout
*************************************************************************************/
link_type *q_next(queue_type *queue_p, link_type *link_p)
{
    if (NULL == queue_p || NULL == link_p)
    {
        DBG_TRACE(DBG_LOG,"return q_next NULL");
        return NULL;
    }

    if (link_p->next == &queue_p->qlink)
    {
        return NULL;
    }
    else
    {
        return link_p->next;
    }
}

/************************************************************************************
* Function: q_delete
* Author @ Date: John.Wang@20200409
* Input:
* Return: 
* Description: delete del_item in queue.
*************************************************************************************/
int q_delete(queue_type *queue_p, link_type *del_item)
{   
    if (NULL == queue_p || 0 == queue_p->len || NULL == del_item)
    {
        DBG_TRACE(DBG_LOG,"return q_delete -1");
        return -1;
    }
    del_item->pre->next = del_item->next;
    del_item->next->pre = del_item->pre;
    queue_p->len--;

    return queue_p->len;
}

/************************************************************************************
* Function: q_size
* Author @ Date: John.Wang@20200409
* Input:
* Return: 
* Description: get the length of the queue
*************************************************************************************/
int q_size(queue_type *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE(DBG_LOG,"return q_size NULL");
        return 0;
    }

    return queue_p->len;
}

/************************************************************************************
* Function: ppkg_init_circal_buff
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_init_circal_buff(circal_buffer *buff_p, char* mem_block)
{
    if (NULL == buff_p)
    {
        DBG_TRACE(DBG_LOG,"return buff_p NULL");
        return;
    }

	buff_p->data = mem_block;
	buff_p->rd_pos = buff_p->wr_pos = 0;
	buff_p->valid_bytes = 0;

	memset(mem_block, 0, MAX_MEM_BLOCK_LEN);
}

/************************************************************************************
* Function: ppkg_reset_circal_buff
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_reset_circal_buff(circal_buffer *buff_p)
{
    if (NULL == buff_p)
    {
        DBG_TRACE(DBG_LOG,"return buff_p NULL");
        return;
    }

	buff_p->rd_pos = buff_p->wr_pos = buff_p->valid_bytes = 0;
	memset(buff_p->data, 0, MAX_MEM_BLOCK_LEN);
}

/************************************************************************************
* Function: ppkg_circal_buff_get_valid_space
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_circal_buff_get_valid_space(circal_buffer *cir_p)
{
    if (cir_p == NULL)
    {
        DBG_TRACE(DBG_LOG,"return cir_p || buff_p NULL");
        return 0;
    }

    return MAX_MEM_BLOCK_LEN - cir_p->valid_bytes;
}

/************************************************************************************
* Function: ppkg_circal_buff_get_valid_bytes
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_circal_buff_get_valid_bytes(circal_buffer *cir_p)
{
    if (cir_p == NULL)
    {
        DBG_TRACE(DBG_LOG,"return cir_p || buff_p NULL");
        return 0;
    }

    return cir_p->valid_bytes;
}

/************************************************************************************
* Function: ppkg_write_circal_buff
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_write_circal_buff(circal_buffer *cir_p, char *buff_p, unsigned int len)
{
    int first_len = 0;
    char *wr_p = NULL;
    
    if (NULL == buff_p || cir_p == NULL)
    {
        DBG_TRACE(DBG_LOG,"return cir_p || buff_p NULL");
        return -1;
    }
    wr_p = &cir_p->data[cir_p->wr_pos];

    if (len > MAX_MEM_BLOCK_LEN || len + cir_p->valid_bytes > MAX_MEM_BLOCK_LEN)
    {
        DBG_TRACE(DBG_LOG,"return not enough space for writing.");
        return -2;
    }

    if (len + cir_p->wr_pos > MAX_MEM_BLOCK_LEN)
    {
        first_len = MAX_MEM_BLOCK_LEN - cir_p->wr_pos;
        memcpy(wr_p, buff_p, first_len);
        cir_p->wr_pos = len - first_len;
        memcpy(&cir_p->data[0], (void *)(buff_p + first_len), cir_p->wr_pos);
    }
    else
    {
        memcpy(wr_p, buff_p, len);
        cir_p->wr_pos += len;
    }

    cir_p->valid_bytes += len;
    DBG_TRACE(DBG_LOG,"valid_bytes:%d, len:%d, [wr:%d, rd:%d]"
        , cir_p->valid_bytes, len
        , cir_p->wr_pos, cir_p->rd_pos);

    return len;
}

/************************************************************************************
* Function: ppkg_read_circal_buff
* Author @ Date: John.Wang@20200216
* Input:    
* Return:   real length of this time reading
* Description: read data from circal buffer
*************************************************************************************/
static int ppkg_read_circal_buff(circal_buffer *cir_p, char *buff_p, unsigned int size)
{
    int read_len = 0;
    char *rd_p = NULL;
    
    if (NULL == buff_p || cir_p == NULL)
    {
        DBG_TRACE(DBG_LOG,"return cir_p || buff_p NULL");
        return -1;
    }
    rd_p = &cir_p->data[cir_p->rd_pos];

    if (cir_p->valid_bytes == 0)
    {
        DBG_TRACE(DBG_LOG,"return no data for reading");
        return -2;
    }

    if (cir_p->wr_pos > cir_p->rd_pos)
    {
        read_len = (cir_p->valid_bytes >= size) ? size : cir_p->valid_bytes;
        memcpy(buff_p, rd_p, read_len);
        cir_p->rd_pos += read_len;
        cir_p->valid_bytes -= read_len;
    }
    else
    {
        int first_len = (MAX_MEM_BLOCK_LEN - cir_p->rd_pos >= size) ? size : (MAX_MEM_BLOCK_LEN - cir_p->rd_pos);
        memcpy(buff_p, rd_p, first_len);
        cir_p->valid_bytes -= first_len;

        if (MAX_MEM_BLOCK_LEN - cir_p->rd_pos < size)
        {
            read_len = cir_p->wr_pos > size - first_len ? size - first_len : cir_p->wr_pos;
            memcpy(buff_p, &cir_p->data[0], read_len);
            cir_p->rd_pos = read_len;
            cir_p->valid_bytes -= read_len;
            read_len += first_len;
        }
        else
        {
            cir_p->rd_pos += first_len;
            read_len = first_len;
        }
    }
    DBG_TRACE(DBG_LOG,"valid_bytes:%d, read_len:%d, [wr:%d, rd:%d]"
        , cir_p->valid_bytes, read_len
        , cir_p->wr_pos, cir_p->rd_pos);

    return read_len;
}

/************************************************************************************
* Function: ppkg_read_circal_buff_line
* Author @ Date: John.Wang@20200216
* Input:    
* Return:   real length of this time reading
* Description: read a string line end with '/r', '/r/n' or '/n'
*************************************************************************************/
static int ppkg_read_circal_buff_line(circal_buffer *cir_p, char *buff_p, unsigned int size)
{
    int i = 0;
    char *rd_p = NULL;
    int bytes_to_search = 0; 
    bool complete_line = FALSE;

    if (NULL == buff_p || cir_p == NULL)
    {
        DBG_TRACE(DBG_LOG,"return cir_p || buff_p NULL");
        return -1;
    }

    if (cir_p->valid_bytes == 0)
    {
        DBG_TRACE(DBG_LOG,"return no data for reading");
        return -2;
    }
    bytes_to_search = cir_p->valid_bytes;
    i = cir_p->rd_pos;
    DBG_TRACE(DBG_LOG,"rd_pos:%d, bytes_to_search:%d", i, bytes_to_search);
    
    while(bytes_to_search-- > 0)
    {
        rd_p = &cir_p->data[i % MAX_MEM_BLOCK_LEN];
        if (*rd_p == '\r' || *rd_p == '\n')
        {
            complete_line = TRUE;
            break;
        }
        i++;
    }

    if (cir_p->data[(i+1) % MAX_MEM_BLOCK_LEN] == '\r' || 
        cir_p->data[(i+1) % MAX_MEM_BLOCK_LEN] == '\n')
    {
        bytes_to_search--;
    }

    DBG_TRACE(DBG_LOG,"complete_line:%d, vbytes:%d, bytes_to_search:%d"
        , complete_line, cir_p->valid_bytes, bytes_to_search);
    
    if (!complete_line)
    {
        return -3;
    }
    else if (cir_p->valid_bytes - bytes_to_search > size)
    {
        DBG_TRACE(DBG_LOG,"return not enough space for a line");
        return -4;
    }

    return ppkg_read_circal_buff(cir_p, buff_p, (cir_p->valid_bytes - bytes_to_search));
}

/************************************************************************************
* Function: ppkg_get_cfg_info
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: parse config file and get config information: path of param file, 
*              firmware version, etc...
*************************************************************************************/
static bool ppkg_get_cfg_info(cfg_info_struct *cfg_info_p)
{
    if (NULL == cfg_info_p)
    {
        DBG_TRACE(DBG_LOG,"return cfg_info_p NULL");
        return FALSE;
    }

    memset(cfg_info_p, 0, sizeof(cfg_info_struct));
    
    //strcpy(cfg_info_p->fm_version, "GV300NR00A17V21M128");
    strcpy(cfg_info_p->fm_version, "GV300NR00A04V07M128");
    strcpy(cfg_info_p->path_def_cfg, "D:\\forfun\\paramPackage_test\\A04V07_default.gv300");
    strcpy(cfg_info_p->path_cust_cfg, "D:\\forfun\\paramPackage_test\\C_GV300_A04V07_M0_0.gv300");
    strcpy(cfg_info_p->path_cust_ini, "D:\\forfun\\paramPackage_test\\C_GV300_A04V07_M0_0_at.ini");
    strcpy(cfg_info_p->password, "gv300");

    DBG_TRACE(DBG_LOG,"cfg_info_p->fm_version:%s", cfg_info_p->fm_version);
    
    return TRUE;
}

/************************************************************************************
* Function: ppkg_read_file
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_read_file(FILE *fp, circal_buffer *cir_buf)
{
    int ret = 0;
    int read_len = 0;

    if (NULL == fp || NULL == cir_buf)
    {
        return -1;
    }
    memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
    
    do 
    {
        read_len = ppkg_circal_buff_get_valid_space(cir_buf) < MAX_TEMP_BLOCK_LEN ? 
                    ppkg_circal_buff_get_valid_space(cir_buf) : MAX_TEMP_BLOCK_LEN;

        if (read_len == 0)
        {
            DBG_TRACE(DBG_LOG,"circal buffer is full");
            ret = -3;
            break;
        }

        read_len = fread(temp_buff, 1, read_len, fp);
        DBG_TRACE(DBG_LOG,"read_len:%d", read_len);
        if (MAX_TEMP_BLOCK_LEN == read_len)
        {
            ppkg_write_circal_buff(cir_buf, temp_buff, MAX_TEMP_BLOCK_LEN);
            memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
        }
        else
        {
            ppkg_write_circal_buff(cir_buf, temp_buff, read_len);
            ret = -3;
            DBG_TRACE(DBG_LOG,"File read complete");
            break;
        }
    }
    while(1);

    return ret;
}

/************************************************************************************
* Function: ppkg_covert_to_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_get_cmd_type(char *cmd_type, char *cmd_str, line_type_enum line_type)
{
    int ret_val = 0;

    if (CFG_LINE == line_type)
    {
        /* <Command Type="GTPIN" EnableAutoUnlockPIN="0" PIN="" /> */
        memcpy(cmd_type, (void*)(cmd_str + CMD_FRAME_HEAD_LEN + 1 + CMD_PREFIX_LEN), MAX_LEN_CMD_TYPE);
        ret_val = (CMD_FRAME_HEAD_LEN + 1 + CMD_PREFIX_LEN);
    }
    else if (CMD_LINE == line_type)
    {
        /* AT+GTPIN=gv300,0,,,,,,,FFFF$ */
        memcpy(cmd_type, (void*)(cmd_str + CMD_ATGT_HEAD_LEN), MAX_LEN_CMD_TYPE);
        ret_val = 0;
    }

    return ret_val;
}

/************************************************************************************
* Function: ppkg_get_buff_line_type
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static line_type_enum ppkg_get_buff_line_type(const char *cmd_str, int cmd_len)
{
    line_type_enum line_type = UNKOWN_LINE;

    if(cmd_len < CMD_FRAME_HEAD_LEN)
    {
        return line_type;
    }

    if (NULL != strstr(cmd_str, LINE_VERSION_FLAG))
    {
        line_type = VER_LINE;
    }
    else if (NULL != strstr(cmd_str, LINE_CMD_FLAG))
    {
        line_type = CFG_LINE;
    }
    else if (NULL != strstr(cmd_str, LINE_ATGT_CMD))
    {
        line_type = CMD_LINE;
    }
    else
    {
    }

    DBG_TRACE(DBG_LOG,"line_type:%d", line_type);
    
    return line_type;
}

/************************************************************************************
* Function: ppkg_get_multi_cmd_id_position
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_get_multi_cmd_id_position(const char* cmd_type)
{
    int i = 0;
    int ret_val = 0;
    
    while (NULL != command_attr_table[i].cmd_name)
    {
        if (strcmp(command_attr_table[i].cmd_name, cmd_type) == 0)
        {
            ret_val = command_attr_table[i].id_pos;
            break;
        }
        i++;
    }

    return ret_val;
}

/************************************************************************************
* Function: ppkg_get_cmd_attr
* Author @ Date: John.Wang@20200305
* Input:
* Return: 
* Description: 
*************************************************************************************/
static cmd_attribute_struct *ppkg_get_cmd_attr(const char* cmd_type)
{
    int i = 0;
    cmd_attribute_struct *ret_p = NULL;
    
    while (NULL != command_attr_table[i].cmd_name)
    {
        if (strcmp(command_attr_table[i].cmd_name, cmd_type) == 0)
        {
            ret_p = &command_attr_table[i];
            break;
        }
        i++;
    }
    DBG_TRACE(DBG_LOG,"i:%d, cmd_type:%s, ret_p:%p", i, cmd_type, ret_p);

    return ret_p;
}


/************************************************************************************
* Function: ppkg_get_multi_cmd_id
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_get_multi_cmd_id(char *cmd_str, int cmd_len, int id_pos)
{
    char para_buff[8] = {0};
    char *p,*q, *k;
    int i = 0;

    p = cmd_str;
    q = k = NULL;
    while(NULL != (q = strchr(p, COMMA_SEPARATOR)))
    {
        /* AT+GTFSC=gv300,,id,20,0,100,,0,,,,,,,,,FFFF$ */
        if (i < id_pos)
        {
            p = q + 1;
            k = p;
            i++; 
        }
        else
        {
            break;
        }
    }

    if (k && q - k > 0)
    {
        memcpy(para_buff, (void*)k, q - k);
        return atoi(para_buff);
    }
    else
    {
        return -1;
    }
}

/************************************************************************************
* Function: ppkg_covert_to_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_covert_to_list(circal_buffer *cir_buf, queue_type *queue_p)
{
    cmd_node_struct *node_p = NULL;
    line_type_enum line_type = UNKOWN_LINE;
    int parsed_len = 0;
    int ret = 0;
    int id_pos = 0;

    if (NULL == cir_buf || NULL == queue_p)
    {
        DBG_TRACE(DBG_LOG,"return path, queue_p NULL");
        return -1;
    }
    
    memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
    ret = ppkg_read_circal_buff_line(cir_buf, temp_buff, MAX_TEMP_BLOCK_LEN);
    line_type = ppkg_get_buff_line_type(temp_buff, ret);
    if (CFG_LINE == line_type || CMD_LINE == line_type)
    {
        if ((node_p = (cmd_node_struct *)malloc(sizeof(cmd_node_struct) + ret)) == NULL)
        {
            DBG_TRACE(DBG_LOG,"malloc failed");
            return -1;
        }
        memset(node_p, 0, (sizeof(cmd_node_struct) + ret));
        parsed_len = ppkg_get_cmd_type(node_p->cmd_type, temp_buff, line_type);
        id_pos = ppkg_get_multi_cmd_id_position(node_p->cmd_type);
        node_p->is_multi_cmd = (bool)(id_pos != 0);
        if (CFG_LINE == line_type)
        {
            /* Skip command type and right " */
            parsed_len += 4;
        }
        node_p->cmd_len = ret - parsed_len;
        memcpy(node_p->cmd_str, &temp_buff[parsed_len], node_p->cmd_len);
        if (CMD_LINE == line_type && node_p->is_multi_cmd)
        {
            node_p->id = ppkg_get_multi_cmd_id(node_p->cmd_str, node_p->cmd_len, id_pos);
        }
        DBG_TRACE(DBG_LOG,"->cmd_type:%s, %d, %s", node_p->cmd_type, node_p->cmd_len, node_p->cmd_str);
        q_put(queue_p, (link_type *)node_p);
    }
    else if (VER_LINE == line_type)
    {
        /* Maybe used for comfire the version */
    }
    else
    {
        DBG_TRACE(DBG_LOG,"unknow line type");
    }

    return ret;
}

/************************************************************************************
* Function: ppkg_build_cmd_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: parse parameter file and build a command list, include command type 
*              and command parameter.
*************************************************************************************/
static int ppkg_build_cmd_list(char* path, queue_type *queue_p)
{
    FILE *read_fp;
    int ret = 0;
    
    if (NULL == path || NULL == queue_p)
    {
        DBG_TRACE(DBG_LOG,"return path, queue_p NULL");
        return -1;
    }

    if ((read_fp = fopen(path, B_FILE_READ_ONLY)) == NULL)
	{
		return -2;
	}
	else
	{
		DBG_TRACE(DBG_LOG,"path=%s, read_fp=%x \n", path, read_fp);
	}

    fseek(read_fp, 0, SEEK_SET);
    do
    {
        if (ppkg_covert_to_list(&ppkg_cntx_p->cir_buff, queue_p) < 0 && -3 != ret)
        {
            ret = ppkg_read_file(read_fp, &ppkg_cntx_p->cir_buff);
        }
    }
    while (ppkg_circal_buff_get_valid_bytes(&ppkg_cntx_p->cir_buff));
    
    ret = q_size(queue_p);
    DBG_TRACE(DBG_LOG,"data convertion complete, rest bytes:%d, queue_p->len:%d"
                    , ppkg_circal_buff_get_valid_bytes(&ppkg_cntx_p->cir_buff), ret);
    ppkg_reset_circal_buff(&ppkg_cntx_p->cir_buff);

    if (NULL != read_fp)
	    fclose(read_fp);
    
    return ret;
}

/************************************************************************************
* Function: ppkg_is_ignore_cmd
* Author @ Date: John.Wang@20200224
* Input:
* Return: TRUE: is ignore command FALSE is not...
* Description: 
*************************************************************************************/
static bool ppkg_is_ignore_cmd(const char* cmd_type)
{
    int i = 0;
    bool ret = FALSE;
    
    while (NULL != command_attr_table[i].cmd_name)
    {
        if (strcmp(command_attr_table[i++].cmd_name, cmd_type) == 0)
        {
            ret = command_attr_table[i++].ignored;
            break;
        }
    }
    //DBG_TRACE(DBG_LOG,"ret:%d, [%s,%s]", ret, multi_cmd_type[i], cmd_type);    

    return ret;
}

/************************************************************************************
* Function: ppkg_is_multi_cmd
* Author @ Date: John.Wang@20200412
* Input:
* Return: TRUE: is multi-command FALSE is not...
* Description: check is multi command by given command type
*************************************************************************************/
static bool ppkg_is_multi_cmd(const char* cmd_type)
{
    int i = 0;
    bool ret = FALSE;
    
    while (NULL != command_attr_table[i].cmd_name)
    {
        if (strcmp(command_attr_table[i++].cmd_name, cmd_type) == 0)
        {
            ret = (bool)(command_attr_table[i++].id_pos != 0);
            break;
        }
    }
    //DBG_TRACE(DBG_LOG,"ret:%d, [%s,%s]", ret, multi_cmd_type[i], cmd_type);    

    return ret;
}

/************************************************************************************
* Function: ppkg_get_key_word
* Author @ Date: John.Wang@20200306
* Input:
* Return: length of key word, 0 means no key word found
* Description: get key word from key_paras, key words is seperated by ','
*************************************************************************************/
static int ppkg_get_key_word(char *key, const char* key_paras)
{
    char *end_pos = NULL;
    int len = 0;

    if (0 == strlen(key_paras)) return 0;

    end_pos = strchr(key_paras, COMMA_SEPARATOR);
    if (end_pos)
    {
        len = end_pos - key_paras;
    }
    else
    {
        /* Not found ',' means only one word */
        len = strlen(key_paras);
    }
    
    if (len) memcpy(key, key_paras, len);

    return len;
}

/************************************************************************************
* Function: ppkg_insert_command_id
* Author @ Date: John.Wang@20200306
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_insert_command_id(int id, cmd_diff_data *diff_data)
{
    int i = 0;
    
    for (; i < MAX_ID_LIST_LEN; i++)
    {
        if (i >= diff_data->diff_id_num)
        {
            diff_data->diff_id_list[diff_data->diff_id_num] = id;
            diff_data->diff_id_num++;
            DBG_TRACE(DBG_LOG,"insert_command_id:%d", id);
            break;
        }
        if (id == diff_data->diff_id_list[i]) break;
    }
}

/************************************************************************************
* Function: ppkg_cmd_keyword_compare
* Author @ Date: John.Wang@20200305
* Input:
* Return: id of sub command, for
* Description: 
*************************************************************************************/
static int ppkg_get_key_param_info(char *cmd_str, 
                    const char *key, int *id, char *key_value)
{
    char *p, *q, *k;
    int key_len = strlen(key);
    char id_str[8] = {0};

    q = k = p = cmd_str;

    /* MinThreshold1="0" 
     * MaxSpeed="0"
     */
    while(NULL != (q = strstr(p, key)))
    {
        /* For example: keyword is "Mode" we found "FRIMode0" 
         * this word should be skipped
         */
        if (*(q - 1) != ' ')
        {
            p = q + key_len;
            continue;
        }

        q += key_len;
        k = strchr(q, '=');
        if (k && k - q > 0)
        {
            memcpy(id_str, q, k - q);
            if (id) *id = atoi(id_str);
        }
        else 
        {
            if (id) *id = -1;
        }

        /* get key value, skip = */
        q = k + 1;
        k = strchr(q + 1, '"');
        if (k && k - q > 0)
        {
            /* Include the last " */
            k++;
            memcpy(key_value, q, k - q); 
        }

        break;
    }

    return k - cmd_str;
}

/************************************************************************************
* Function: ppkg_get_param_value
* Author @ Date: John.Wang@20200428
* Input:
* Return: 
* Description: length of para_value
*************************************************************************************/
static int ppkg_get_param_value(char *src_str, char *param, char *para_value)
{
    /* GEOID="5" */
    char *p = strstr(src_str, param);
    char *q = NULL;

    if (NULL == p)
    {
        return 0;
    }

    p += (strlen(param) + 2); /* Skip '=' and left '"' */
    q = p;
    p = strchr(q, '"'); /* find right '"'*/
    if (p != NULL)
    {
        *p = (char)0;
        memcpy(para_value, q, p - q);
    }
    else
    {
        return 0;
    }

    return p - q;
}

/************************************************************************************
* Function: ppkg_cmd_keyword_compare
* Author @ Date: John.Wang@20200305
* Input:
* Return: id of sub command, for
* Description: 
*************************************************************************************/
static bool ppkg_cmd_keyword_compare(char *key, char* cmd_type, char *def_str,
                        char *cust_str, cmd_diff_data *diff_data)
{
    int id = -1;
    char def_value[MAX_TEMP_BUFF_LEN] = {0};
    char cust_value[MAX_TEMP_BUFF_LEN] = {0};
    char id_str[8] = {0};
    int l1, l2;
    int def_len = 0;
    int cust_len = 0;
    bool ret = TRUE;

    while((l1 = ppkg_get_key_param_info((char*)(def_str + def_len), key, NULL, def_value)) > 0 && 
          (l2 = ppkg_get_key_param_info((char*)(cust_str + cust_len), key, &id, cust_value)) > 0)
    {
        if (strcmp("PEO", cmd_type) == 0 &&
            ppkg_get_param_value((cust_str + cust_len), "GEOID", id_str) != 0
            )
        {
            id = atoi(id_str);
        }
        def_len += l1;
        cust_len += l2;
        DBG_TRACE(DBG_LOG,"def_len:%d, cust_len:%d, def_value:%s, cust_value:%s, id:%d"
            , def_len, cust_len, def_value, cust_value, id);
        if (strcmp(def_value, cust_value))
        {
            if (id >= 0)
                ppkg_insert_command_id(id, diff_data);

            ret = FALSE;
        }
        memset(def_value, 0, MAX_TEMP_BUFF_LEN);
        memset(cust_value, 0, MAX_TEMP_BUFF_LEN);
    }

    return ret;
}

/************************************************************************************
* Function: ppkg_get_multi_cmd_str
* Author @ Date: John.Wang@20200307
* Input:
* Return: 
* Description: 
*************************************************************************************/
static bool ppkg_get_multi_cmd_str(char *flag, char *sub_str, int *id, char *cmd_str)
{
    char *p, *q, *k;
    int flag_len = strlen(flag);
    char id_str[8] = {0};
    int ret_len = 0;
    
    q = p = cmd_str;
    k = NULL;

    /* MinThreshold1="0" 
     * MaxSpeed="0"
     */
    while(NULL != (q = strstr(p, flag)))
    {
        /* For example: keyword is "Mode" we found "FRIMode0" 
         * this word should be skipped
         */
        if (*(q - 1) != ' ')
        {
            p = q + flag_len;
            continue;
        }

        if (NULL == k)
        {
            p = q + flag_len;
            k = q; /* record the head of sub_str to k */
        }
        else
        {
            /* means we already found the head of sub_str to k */
            break;
        }
    }
   
    if (k)
    {
        /* found head */
        if (q)
        {
            /* found next sub_str head */
            ret_len = (int)(q - k);
        }
        else
        {
            /* not found next sub_str head, so we conside the total cmd_str as a sub_str */
            ret_len = strlen((const char*)k);
        }
        memcpy(sub_str, k, ret_len);

        /* pick out ID, GEOID0="0" */
        q = k + flag_len;
        k = strchr(q, '=');
        if (k && k - q > 0)
        {
            memcpy(id_str, q, k - q);
            if (id) *id = atoi(id_str);
        }
        else 
        {
            if (id) *id = -1;
        }
    }
    else
    {
        /* not found head, we got nothing from cmd_str */
        ret_len = 0;
    }
    
    return ret_len;
}

/************************************************************************************
* Function: ppkg_cmd_compare
* Author @ Date: John.Wang@20200305
* Input:
* Return: TRUE: is the same command FALSE is not...
* Description: 
*************************************************************************************/
static bool ppkg_cmd_compare(cmd_node_struct *def_node
                    , cmd_node_struct *cust_node, cmd_diff_data *diff_data)
{
    int key_word_len = 0;
    int parsed_len = 0;
    int id = -1;
    bool ret = TRUE;
    char *key_word = NULL;
    char id_str[8] = {0};
    cmd_attribute_struct *cmd_attr_p = ppkg_get_cmd_attr(cust_node->cmd_type);

    if (NULL == cmd_attr_p || NULL == cmd_attr_p->key_paras)
    {
        int clen = 0;
        int dlen = 0;
        int cparse_len = 0;
        int dparse_len = 0;
        char dsub_str[MAX_TEMP_BUFF_LEN] = {0};
        char csub_str[MAX_TEMP_BUFF_LEN] = {0};
       
        if (!cust_node->is_multi_cmd || !cmd_attr_p || !cmd_attr_p->first_param)
        {
            ret = (bool)(0 == strcmp(cust_node->cmd_str, def_node->cmd_str));
            diff_data->diff_id_list = NULL;
            diff_data->diff_id_num = MAX_COMMAND_NUM;
        }
        else
        {
            DBG_TRACE(DBG_LOG,"is_multi_cmd:%d, first_param:%p, cust_str_len:%d, def_str_len:%d"
                , cust_node->is_multi_cmd, cmd_attr_p->first_param
                , strlen(cust_node->cmd_str), strlen(def_node->cmd_str)
                );
            while (cparse_len < strlen(cust_node->cmd_str) && 
                   dparse_len < strlen(def_node->cmd_str))
            {
                clen = ppkg_get_multi_cmd_str(cmd_attr_p->first_param
                        , csub_str, &id, (char*)(cust_node->cmd_str + cparse_len));
                dlen = ppkg_get_multi_cmd_str(cmd_attr_p->first_param
                        , dsub_str, NULL, (char*)(def_node->cmd_str + dparse_len));
                if (!clen || !dlen)
                {
                    break;
                }
                cparse_len += clen;
                dparse_len += dlen;

                if (strcmp("PEO", cust_node->cmd_type) == 0 &&
                    ppkg_get_param_value(csub_str, "GEOID", id_str) != 0
                    )
                {
                    id = atoi(id_str);
                }

                if (clen != dlen || 
                    strcmp(csub_str, dsub_str))
                {
                    if (id >= 0)
                        ppkg_insert_command_id(id, diff_data);
                    ret = FALSE;
                }

                memset(csub_str, 0, MAX_TEMP_BUFF_LEN);
                memset(dsub_str, 0, MAX_TEMP_BUFF_LEN);
            }
        }
    }
    else
    {
        key_word = (char*)malloc(strlen(cmd_attr_p->key_paras));
        if(NULL == key_word) return ret;
        
        /* Key parameters compare */
        do
        {
            memset(key_word, 0, strlen(cmd_attr_p->key_paras));
            key_word_len = ppkg_get_key_word(key_word, (const char *)(cmd_attr_p->key_paras + parsed_len));
            *(key_word + key_word_len) = (char)0;
            DBG_TRACE(DBG_LOG,"key_word_len:%d, parsed_len:%d, key_word:%s", key_word_len, parsed_len, key_word);
            
            if (key_word_len)
            {
                /* Skip ',' */
                key_word_len++;
                parsed_len += key_word_len;
                
                ret = ppkg_cmd_keyword_compare(key_word, 
                            cust_node->cmd_type,
                            def_node->cmd_str,
                            cust_node->cmd_str,
                            diff_data);
            }
            else
            {
                break;
            }
        }
        while(parsed_len < strlen(cmd_attr_p->key_paras));

        free(key_word);
    }
    //DBG_TRACE(DBG_LOG,"ret:%d, [%s,%s]", ret, multi_cmd_type[i], cmd_type);    

    return ret;
}


/************************************************************************************
* Function: ppkg_cmp_list_cb_diff
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: a callback function to compare between default configuration and customer 
*              configuration, then generate a diff command list.
*************************************************************************************/
static int ppkg_cmp_list_cb_diff(queue_type *def_q
            , link_type *def_ptr, link_type *cust_ptr)
{
    diff_cmd_node   *diff_node = NULL;
    cmd_node_struct *def_node = (cmd_node_struct *)def_ptr;
    cmd_node_struct *cust_node = (cmd_node_struct *)cust_ptr;
    cmd_diff_data   diff_data = {0};
    queue_type      *differ_cfg_queue;

    diff_data.diff_id_num = 0;
    diff_data.diff_id_list = &command_id_list[0];
    differ_cfg_queue = &ppkg_cntx_p->differ_cfg;

    do
    {
        if (ppkg_is_ignore_cmd(cust_node->cmd_type))
        {
            DBG_TRACE(DBG_LOG,"ignore command");
            break;
        }
        //DBG_TRACE(DBG_LOG,"ppkg_cmp_list_cb_diff[%s, %s]", cust_node->cmd_type, def_node->cmd_type);
        if (0 == strcmp(cust_node->cmd_type, def_node->cmd_type))
        {
            if (!ppkg_cmd_compare(def_node, cust_node, &diff_data))
            {
                if ((diff_node = (diff_cmd_node *)malloc(sizeof(diff_cmd_node))) == NULL)
                {
                    DBG_TRACE(DBG_LOG,"malloc failed");
                    break;
                }
                memset(diff_node, 0, (sizeof(diff_cmd_node)));
                strcpy(diff_node->cmd_type, cust_node->cmd_type);
                DBG_TRACE(DBG_LOG,"is_multi_cmd:%d, diff_id_list:%p"
                        , cust_node->is_multi_cmd, diff_data.diff_id_list);
                if (cust_node->is_multi_cmd && diff_data.diff_id_list)
                {
                    if (NULL == (diff_node->diff_data.diff_id_list 
                                    = (int *)malloc(sizeof(int) * diff_data.diff_id_num)))
                    {
                        DBG_TRACE(DBG_LOG,"malloc failed2");
                        break;
                    }

                    DBG_TRACE(DBG_LOG,"diff_data.diff_id_num:%d", diff_data.diff_id_num);
                    diff_node->diff_data.diff_id_num = diff_data.diff_id_num;
                    memset(diff_node->diff_data.diff_id_list
                            , 0, (sizeof(int) * diff_data.diff_id_num));
                    memcpy(diff_node->diff_data.diff_id_list
                            , diff_data.diff_id_list
                            , (sizeof(int) * diff_data.diff_id_num));
                }
                
                DBG_TRACE(DBG_LOG,"->diff_node:%s", diff_node->cmd_type);
                q_put(differ_cfg_queue, (link_type*)diff_node);
            }
            
            if (q_delete(def_q, (link_type *)def_node) >= 0)
            {
                free(def_node);  
            }
            break;
        }
        else
        {
            //DBG_TRACE(DBG_LOG,"ppkg_cmp_list_cb_diff[%p, %p]", def_node, def_node->qlink.next);
            def_node = (cmd_node_struct *)q_next(def_q, (link_type *)def_node);
        }
    }
    while (def_node != NULL);

    return q_size(differ_cfg_queue);
}

/************************************************************************************
* Function: ppkg_release_queue
* Author @ Date: John.Wang@20200413
* Input:
* Return:
* Description:
*************************************************************************************/
static void ppkg_release_queue(queue_type *queue_p)
{
    link_type *p_node = NULL;
    do
    {
        p_node = q_check(queue_p);
        if (NULL == p_node)
        {
            break;
        }
        free(p_node);
    }
    while (q_size(queue_p) > 0);
    q_destroy(queue_p);
}


/************************************************************************************
* Function: ppkg_compare_cmd_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: compare between compare queue and operation queue, then 
*              generate a diff queue.
*************************************************************************************/
static int ppkg_compare_cmd_list(
            queue_type *cmp_q, queue_type *opr_q, cb_func cb)
{
    link_type *cmp_node = NULL;
    link_type *opr_node = NULL;
    int ret = 0;
    
    if (NULL == cmp_q || NULL == opr_q)
    {
        DBG_TRACE(DBG_LOG,"return cust_q || def_q");
        return 0;
    }
    
    do
    {
        cmp_node = q_get(cmp_q);
        opr_node = q_check(opr_q);
        if (NULL == opr_node || NULL == cmp_node)
        {
            DBG_TRACE(DBG_LOG,"opr_node:%p, cmp_node:%p", opr_node, cmp_node);
            break;
        }

        //DBG_TRACE(DBG_LOG,"def_node:%p, cust_node:%p", opr_node, cmp_node);
        ret = cb(cmp_q, cmp_node, opr_node);
        
        DBG_TRACE(DBG_LOG,"opr_q_len:%d, cmp_q_len:%d, ret=%d", q_size(opr_q),  q_size(cmp_q), ret);

        free(opr_node);
    }
    while (q_size(opr_q) > 0);

    q_destroy(opr_q);
    ppkg_release_queue(cmp_q);

    DBG_TRACE(DBG_LOG,"ppkg_compare_cmd_list ret:%d", ret);
    
    return ret;
}

/************************************************************************************
* Function: ppkg_write_buffer_to_file
* Author @ Date: John.Wang@20200222
* Input:
* Return: void
* Description:  
*************************************************************************************/
static void ppkg_write_buffer_to_file(FILE *fp, circal_buffer *c_buffer)
{
    if (fp != NULL && c_buffer != NULL && c_buffer->valid_bytes != 0)
        fwrite(c_buffer->data, 1, c_buffer->valid_bytes, fp);
}

/************************************************************************************
* Function: ppkg_assemble_single_command
* Author @ Date: John.Wang@20200222
* Input:
* Return: void
* Description:  assemble atfile 
*************************************************************************************/
static int ppkg_assemble_single_command(cmd_node_struct *ini_node)
{
    circal_buffer *cir_buff_ptr = &ppkg_cntx_p->cir_buff;
    char *p = NULL;
    int mlc_size = 0;
    int len = 0;

    mlc_size = ini_node->cmd_len + MAX_ATFILE_ITEM_FMT_LEN;
    
    if ((p = (char *)malloc(mlc_size)) == NULL)
    {
        DBG_TRACE(DBG_LOG,"malloc failed");
        return 0;
    }
    DBG_TRACE(DBG_LOG,"ppkg_assemble_single_command p:%p, mlc_size=%d, cmd_type:%s"
        , p, mlc_size, ini_node->cmd_type);
    memset(p, 0, mlc_size);
    
    len = snprintf(p, mlc_size, "AtCmd[%d]=%s", 
                ppkg_cntx_p->cmd_cnt, ini_node->cmd_str);
    len += snprintf((char*)(p + len), mlc_size - len,
                "MetaResult[%d]=OK\r\n\r\n",
                ppkg_cntx_p->cmd_cnt);
    ppkg_cntx_p->cmd_cnt++;
    DBG_TRACE(DBG_LOG,"->atfile line:%s, len=%d", p, len);
    if (ppkg_circal_buff_get_valid_space(cir_buff_ptr) < len)
    {
        /* not enough space for buffer */
        ppkg_write_buffer_to_file(ppkg_cntx_p->temp_fp, cir_buff_ptr);
        ppkg_reset_circal_buff(cir_buff_ptr);
    }
    ppkg_write_circal_buff(cir_buff_ptr, p, len);

    free(p);

    return len;
}

/************************************************************************************
* Function: ppkg_cmp_list_cb_atfile
* Author @ Date: John.Wang@20200222
* Input:
* Return: length of atfile
* Description: 
*************************************************************************************/
static int ppkg_cmp_list_cb_atfile(queue_type *ini_q
                , link_type *ini_ptr, link_type *diff_ptr)
{
    cmd_node_struct *ini_node = (cmd_node_struct *)ini_ptr;
    diff_cmd_node   *diff_node = (diff_cmd_node *)diff_ptr;
    cmd_diff_data   *data_ptr = NULL;
    link_type       *next_ini_ptr = NULL;
    char *first_cmd_type = NULL;
    queue_type      *at_queue_ptr = NULL;
    int i = 0;
    at_queue_ptr = &ppkg_cntx_p->at_queue;

    if (NULL == diff_node || NULL == ini_node)
    {
        DBG_TRACE(DBG_LOG,"diff_node || ini_node");
        return 0;
    }
    
    do
    {
        if (0 == strcmp(diff_node->cmd_type, ini_node->cmd_type))
        {
            if (!ini_node->is_multi_cmd)
            {
                ppkg_assemble_single_command(ini_node);
                if (q_delete(ini_q, (link_type *)ini_node) >= 0)
                {
                    q_put(at_queue_ptr, (link_type *)ini_node);
                }
            }
            else
            {
                first_cmd_type = ini_node->cmd_type;
                DBG_TRACE(DBG_LOG,"first_cmd_type:%s", first_cmd_type);
                data_ptr = &(diff_node->diff_data);
                do
                {
                    next_ini_ptr = q_next(ini_q , (link_type *)ini_node);

                    DBG_TRACE(DBG_LOG,"diff_id_list:%p, next_ini_ptr:%p, ini_node->id:%d, i:%d, diff_id_num:%d, cmd_type:%s"
                        , data_ptr->diff_id_list, next_ini_ptr, ini_node->id, i, data_ptr->diff_id_num
                        , ini_node->cmd_type);

                    if (NULL == data_ptr->diff_id_list || 
                        data_ptr->diff_id_list[i] == ini_node->id)
                    {
                        ppkg_assemble_single_command(ini_node);
                        i++;
                        if (q_delete(ini_q, (link_type *)ini_node) >= 0)
                        {
                            q_put(at_queue_ptr, (link_type *)ini_node);
                        }
                    }
                    ini_node = (cmd_node_struct *)next_ini_ptr;
                }
                while (ini_node != NULL && 
                       i < data_ptr->diff_id_num && 
                       (strcmp(first_cmd_type, ini_node->cmd_type) == 0));
            }
            break;
        }
        else
        {
            ini_node = (cmd_node_struct *)q_next(ini_q, (link_type *)ini_node);
        }
    }
    while (ini_node != NULL);

    return ppkg_cntx_p->cmd_cnt;
}

/************************************************************************************
* Function: ppkg_create_file
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_assemble_file_head(void)
{
    ppkg_write_circal_buff(&ppkg_cntx_p->cir_buff, AT_FILE_HEAD_STR, strlen(AT_FILE_HEAD_STR));
}

/************************************************************************************
* Function: ppkg_create_file
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_assemble_file_tail(void)
{
    char buffer[32] = {0};
    int len = 0;
    
    len = snprintf(buffer, 32, "AtCmd[%d]=%s", ppkg_cntx_p->cmd_cnt, AT_FILE_TAIL_STR);

    ppkg_write_circal_buff(&ppkg_cntx_p->cir_buff, buffer, len);
}

/************************************************************************************
* Function: ppkg_close_file
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_close_file(FILE *fp)
{
    if (NULL != fp)
    {
        fclose(fp);
    }
}

/************************************************************************************
* Function: ppkg_create_file
* Author @ Date: John.Wang@20200222
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_create_file(char *path, ppkg_gen_context *cntx_p)
{
    if (NULL == path)
    {
        return;
    }

    /* if ATFILE.ini is already exist, delete it */
    if(access(path, IS_EXIST) == 0)
    {
        remove(path);
    }

    if ((cntx_p->temp_fp = fopen(path, T_FILE_WRITE_ADD)) == NULL)
	{
		return;
	}
	else
	{
		DBG_TRACE(DBG_LOG,"path=%s, read_fp=%x \n", path, cntx_p->temp_fp);
	}
    
    fseek(cntx_p->temp_fp, 0, SEEK_SET);
}

/************************************************************************************
* Function: ppkg_com_create
* Author @ Date: John.Wang@20200411
* Input:
* Return: TRUE/FALSE
* Description: create com port, set com name, baudrate, stop bit, etc...
*************************************************************************************/
static bool ppkg_com_create(ppkg_gen_context *cntx_p)
{
    if (INVALID_HANDLE_VALUE != cntx_p->com_hdlr)
    {
        return TRUE;
    }
    
    cntx_p->com_hdlr = create_com("com4", 115200, 8);
    if (INVALID_HANDLE_VALUE == cntx_p->com_hdlr)
    {
        DBG_TRACE(DBG_LOG,"open com failed!");
        return FALSE;
    }   

    return TRUE;
}

/************************************************************************************
* Function: ppkg_com_destroy
* Author @ Date: John.Wang@20200411
* Input:
* Return: TRUE/FALSE
* Description: destroy com port, reset com handle
*************************************************************************************/
static void ppkg_com_destroy(ppkg_gen_context *cntx_p)
{
    if (INVALID_HANDLE_VALUE != cntx_p->com_hdlr)
    {
        destroy_com(cntx_p->com_hdlr);
        cntx_p->com_hdlr = INVALID_HANDLE_VALUE;
    }    
}

/************************************************************************************
* Function: ppkg_com_write
* Author @ Date: John.Wang@20200411
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_com_write(ppkg_gen_context *cntx_p, char *wr_buf, int wr_len)
{
    char *buf_ptr = NULL;
    int real_len = 0;
    int cmd_len = 0;

    buf_ptr = (char*)malloc(wr_len + 3);
    if (buf_ptr == NULL)
    {
        return 0;
    }
    memset(buf_ptr, 0, (wr_len + 3));
    cmd_len = snprintf(buf_ptr, wr_len + 3, "%s%s", wr_buf, "\r\n");
    real_len = com_write(cntx_p->com_hdlr, buf_ptr, cmd_len);

    free(buf_ptr);

    return real_len;
}

/************************************************************************************
* Function: ppkg_com_read
* Author @ Date: John.Wang@20200411
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_com_read(ppkg_gen_context *cntx_p, char *rd_buf, int rd_size)
{
    int real_len = 0;
    int rd_len = 0;

    memset(rd_buf, 0, rd_size);
#if 0
    while(MAX_TEMP_BLOCK_LEN - real_len > 0)
    {
        if (com_read(cntx_p->com_hdlr, &rd_buf[real_len], rd_size - real_len, &rd_len))
        {
            real_len += rd_len;
        }
        else
        {
            break;
        }
    }
#else
    if (com_read(cntx_p->com_hdlr, rd_buf, rd_size, &rd_len))
    {
        real_len = rd_len;
    }
#endif
    DBG_TRACE(DBG_LOG,"real_len:%d, %s", real_len, rd_buf);

    return real_len;
}


/************************************************************************************
* Function: ppkg_device_info_check
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: check com communication, device information
*************************************************************************************/
static bool ppkg_device_info_check(ppkg_gen_context *cntx_p, cfg_info_struct *cfg_info_p)
{
    bool ret_val = FALSE;

    if (!ppkg_com_create(cntx_p))
    {
        return ret_val;
    }

    ppkg_com_write(cntx_p, LINE_ATCSUB_CMD, strlen(LINE_ATCSUB_CMD));
    ppkg_com_read(cntx_p, read_buff, MAX_TEMP_BLOCK_LEN);    

    if (NULL != strstr(read_buff, cfg_info_p->fm_version))
    {
        ret_val = TRUE;
    }

    return ret_val;
}

/************************************************************************************
* Function: ppkg_assem_confirm_info_single_line_head
* Author @ Date: John.Wang@20200411
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_assem_confirm_info_single_line_head(
            char *buf_ptr, int buf_size, int cmd_cnt, int sub_cnt)
{
    int  ret_len = 0;

    if (0 == sub_cnt)
    {
        ret_len = snprintf(buf_ptr, buf_size, "MetaResult[%d]=", cmd_cnt);
    }
    else
    {
        ret_len = snprintf(buf_ptr, buf_size, "MetaResult[%d-%d]=", cmd_cnt, sub_cnt);
    }

    return ret_len;
}


/************************************************************************************
* Function: ppkg_assem_confirm_info_single_line
* Author @ Date: John.Wang@20200411
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_assem_confirm_info_single_line_body(
            char *buf_ptr, int buf_size, char *cfm_str, int total_len)
{
    char *p, *q;
    int  comma_skip = 0;
    int  ret_len = 0;

    /* get confirm information 
     * step 1: search the second ',' from head to tail
     * step 2: search the first ',' from tail to head
     */
    /* step 1 */
    p = cfm_str;
    q = NULL;
    while ( comma_skip < 2 && 
            p != NULL &&
            p - cfm_str < total_len
          )
    {
        p = strchr(p, COMMA_SEPARATOR);
        if (NULL == p) break;
        comma_skip++;
        p++;
    }

    /* step 2 */
    if (2 == comma_skip && NULL != p) q = strrchr(p, COMMA_SEPARATOR);
    if (NULL != q)
    {
        *(q + 1) = (char)0;
        ret_len += snprintf((char*)(buf_ptr + ret_len), buf_size - ret_len, "%s", p);
    }

    return ret_len;
}

/************************************************************************************
* Function: ppkg_assem_confirm_info
* Author @ Date: John.Wang@20200412
* Input:
* Return:
* Description:
*************************************************************************************/
static int ppkg_assem_confirm_info(char *buf_ptr, int buf_size, 
                    char *cfm_str, char *cmd_type, int cmd_cnt)
{
    //bool is_multi_cmd = ppkg_is_multi_cmd(cmd_type);
    char *p, *q;
    int  total_len = strlen(cfm_str);
    int  ret_len = 0;
    int  sub_count = 0;
    int  line_len = 0;

    DBG_TRACE(DBG_LOG,"total_len=%d, buf_size:%d", total_len, buf_size);

    p = q = cfm_str;

    /* search for tail of a single line, end by "0x0d 0x0a 0x0d 0x0a" */
    while((p = strchr(q, '\r')) != NULL && p - cfm_str < total_len)
    {
        while ((*p == '\r' || *p == '\n') && p - cfm_str < total_len) p++;

        /* skip query command echo, such as: AT+GTCLT?"gv300" */
        if (strstr(q, "?\"") != NULL)
        {
            q = p;
            continue;
        }

        /* ignore useless contents, like: OK */
        line_len = p - q;
        if (line_len < 7)
        {
            DBG_TRACE(DBG_LOG,"All confirm info packaged");
            break;
        }

        memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
        memcpy(temp_buff, q, line_len);

        ret_len += ppkg_assem_confirm_info_single_line_head(
                        (char*)(buf_ptr + ret_len), 
                        buf_size - ret_len,
                        cmd_cnt,
                        sub_count++
                        );
        ret_len += ppkg_assem_confirm_info_single_line_body(
                        (char*)(buf_ptr + ret_len), 
                        buf_size - ret_len, 
                        temp_buff, 
                        p - q
                        );
        DBG_TRACE(DBG_LOG,"ret_len=%d, buf_size:%d", ret_len, buf_size - ret_len);
        ret_len += snprintf((char*)(buf_ptr + ret_len), buf_size - ret_len, "\r\n");

        q = p;
    }
    ret_len += snprintf((char*)(buf_ptr + ret_len), buf_size - ret_len, "\r\n");

    return ret_len;
}

/************************************************************************************
* Function: ppkg_build_confirm_file
* Author @ Date: John.Wang@20200412
* Input:
* Return:
* Description:
*************************************************************************************/
static bool ppkg_build_confirm_file(ppkg_gen_context *cntx_p, cfg_info_struct *cfg_ptr)
{
    queue_type      *at_queue_ptr = NULL;
    cmd_node_struct *at_node = NULL;
    circal_buffer   *cbuf_ptr = NULL;
    char    query_cmd[MAX_QUERY_CMD_LEN] = {0};
    char    pre_cmd_type[MAX_LEN_CMD_TYPE + 1] = {0};
    char    *mlc_ptr = NULL;
    int     length = 0;
    int     read_len = 0;

    at_queue_ptr = &cntx_p->at_queue;
    cbuf_ptr = &cntx_p->cir_buff;
    ppkg_cntx_p->cmd_cnt = 0;

    do
    {
        at_node = (cmd_node_struct *)q_check(at_queue_ptr);
        if (NULL == at_node)
        {
            break;
        }

        ppkg_com_write(cntx_p, at_node->cmd_str, at_node->cmd_len);
        ppkg_com_read(cntx_p, read_buff, MAX_TEMP_BLOCK_LEN);

        if (0 == strlen(pre_cmd_type))
        {
            strcpy(pre_cmd_type, at_node->cmd_type);
        }
        else if (strcmp(pre_cmd_type, at_node->cmd_type) != 0
                 || 0 == q_size(at_queue_ptr))
        {
            Sleep(100);
            /*AT+GTPEO?"gv300"*/
            length = snprintf(query_cmd, MAX_QUERY_CMD_LEN, "%s%s?\"%s\"", LINE_ATGT_CMD, pre_cmd_type, cfg_ptr->password);
            ppkg_com_write(cntx_p, query_cmd, length);
            read_len = ppkg_com_read(cntx_p, read_buff, MAX_TEMP_BLOCK_LEN);
            if (0 == read_len)
            {
                DBG_TRACE(DBG_LOG,"read failed");
                break;
            }

            if ((mlc_ptr = (char *)malloc(read_len)) == NULL)
            {
                DBG_TRACE(DBG_LOG,"malloc failed");
                break;
            }
            DBG_TRACE(DBG_LOG,"mlc_ptr:%p, mlc_size=%d", mlc_ptr, read_len);
            memset(mlc_ptr, 0, read_len);

            length = snprintf(mlc_ptr, read_len, "AtCmd[%d]=%s\r\n", ppkg_cntx_p->cmd_cnt, query_cmd);

            length += ppkg_assem_confirm_info((char*)(mlc_ptr + length), read_len, read_buff, pre_cmd_type, ppkg_cntx_p->cmd_cnt);
            if (ppkg_circal_buff_get_valid_space(cbuf_ptr) < length)
            {
                /* not enough space for buffer */
                ppkg_write_buffer_to_file(ppkg_cntx_p->temp_fp, cbuf_ptr);
                ppkg_reset_circal_buff(cbuf_ptr);
            }
            ppkg_write_circal_buff(cbuf_ptr, mlc_ptr, length);
            free(mlc_ptr);

            memcpy(pre_cmd_type, at_node->cmd_type, MAX_LEN_CMD_TYPE);

            ppkg_cntx_p->cmd_cnt++;
        }

        free(at_node);
        Sleep(1000);
    }
    while (q_size(at_queue_ptr) > 0);

    q_destroy(at_queue_ptr);

    return TRUE;
}


/************************************************************************************
* Function: ppkg_init_circal_buff
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static void ppkg_init_context(void)
{
    memset(ppkg_cntx_p, 0, sizeof(ppkg_gen_context));
    q_init(&ppkg_cntx_p->default_cfg);
    q_init(&ppkg_cntx_p->custom_cfg);
    q_init(&ppkg_cntx_p->custom_ini);
    q_init(&ppkg_cntx_p->differ_cfg);
    q_init(&ppkg_cntx_p->at_queue);

    ppkg_init_circal_buff(&ppkg_cntx_p->cir_buff, memory_block);
    ppkg_cntx_p->com_hdlr = INVALID_HANDLE_VALUE;
}

/************************************************************************************
* Function: ppkg_gen
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: pre-parser of the config file, parameter file then generator ATFILE and
*              config comfirm file.
*************************************************************************************/
gen_result_enum ppkg_gen(void)
{
    gen_result_enum ret_val = GEN_SUCCESS;
    
    ppkg_init_context();

    /* parse configuration file get config information, such as file path, 
     * firmware version, com name, baudrate etc..
     */
    if (!ppkg_get_cfg_info(&cfg_info))
    {
        DBG_TRACE(DBG_LOG,"return ppkg_get_cfg_info");
        ret_val = GEN_CONFIG_FAIL;
    }
    if (ret_val != GEN_SUCCESS) return ret_val;

    /* check com communication, device information */
    if (!ppkg_device_info_check(ppkg_cntx_p, &cfg_info))
    {
        DBG_TRACE(DBG_LOG,"return ppkg_device_info_check");
        ret_val = GEN_PRE_CHECK;
    }
    if (ret_val != GEN_SUCCESS)
    {
        ppkg_com_destroy(ppkg_cntx_p);
        return ret_val;
    }

    /* build default configuration queue, custom configuration queue and custom command queue */
    if (ppkg_build_cmd_list(cfg_info.path_def_cfg, &ppkg_cntx_p->default_cfg) &&
        ppkg_build_cmd_list(cfg_info.path_cust_cfg, &ppkg_cntx_p->custom_cfg) &&
        ppkg_build_cmd_list(cfg_info.path_cust_ini, &ppkg_cntx_p->custom_ini) >= 0)
    {
        DBG_TRACE(DBG_LOG,"def_cfg->len:%d cust_cfg->len:%d, cust_ini->len:%d",
                q_size(&ppkg_cntx_p->default_cfg), 
                q_size(&ppkg_cntx_p->custom_cfg), 
                q_size(&ppkg_cntx_p->custom_ini));
    }
    else
    {
        ret_val = GEN_CMD_LIST_FAIL;
    }
    if (ret_val != GEN_SUCCESS)
    {
        ppkg_com_destroy(ppkg_cntx_p);
        return ret_val;
    }

    /* compare default configuration queue and custom configuration queue, then generate ATFILE */
    ppkg_create_file(AT_CFG_FILE_NAME, ppkg_cntx_p);
    ppkg_assemble_file_head();
    if (ppkg_compare_cmd_list(&ppkg_cntx_p->default_cfg, &ppkg_cntx_p->custom_cfg, ppkg_cmp_list_cb_diff) &&
        ppkg_compare_cmd_list(&ppkg_cntx_p->custom_ini, &ppkg_cntx_p->differ_cfg, ppkg_cmp_list_cb_atfile))
    {
        DBG_TRACE(DBG_LOG,"build AT file");
        ppkg_assemble_file_tail();
        ppkg_write_buffer_to_file(ppkg_cntx_p->temp_fp, &ppkg_cntx_p->cir_buff);
        ppkg_reset_circal_buff(&ppkg_cntx_p->cir_buff);
    }
    else
    {
        DBG_TRACE(DBG_LOG,"failed to build AT file");
        ret_val = GEN_ATFILE;
    }
    ppkg_close_file(ppkg_cntx_p->temp_fp);
    if (ret_val != GEN_SUCCESS)
    {
        ppkg_com_destroy(ppkg_cntx_p);
        return ret_val;
    }

    /* generate config confirm file */
    ppkg_create_file(AT_CFM_FILE_NAME, ppkg_cntx_p);
    ppkg_assemble_file_head();
    if (ppkg_build_confirm_file(ppkg_cntx_p, &cfg_info))
    {
        DBG_TRACE(DBG_LOG,"build CFM file");
        ppkg_assemble_file_tail();
        ppkg_write_buffer_to_file(ppkg_cntx_p->temp_fp, &ppkg_cntx_p->cir_buff);
    }
    else
    {
        DBG_TRACE(DBG_LOG,"failed to build CFM file");
        ret_val = GEN_CONFIRM;
    }
    ppkg_close_file(ppkg_cntx_p->temp_fp);
    ppkg_com_destroy(ppkg_cntx_p);

    return ret_val;
}

