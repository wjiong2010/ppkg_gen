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
#include <string.h>
#include "debug.h"
#include "ppkg_generator_struct.h"

/************************************************************************************
* Define and Declaration
*************************************************************************************/
static char memory_block[MAX_MEM_BLOCK_LEN] = {0};
static char temp_buff[MAX_TEMP_BLOCK_LEN] = {0};

static cfg_info_struct cfg_info;
static cmd_list_struct cmd_list;
static circal_buffer   cir_buff;

/************************************************************************************
* Process
*************************************************************************************/
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
        DBG_TRACE("return buff_p NULL");
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
        DBG_TRACE("return buff_p NULL");
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
        DBG_TRACE("return cir_p || buff_p NULL");
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
        DBG_TRACE("return cir_p || buff_p NULL");
        return 0;
    }

    return cir_p->valid_bytes;
}

/************************************************************************************
* Function: ppkg_reset_circal_buff
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
        DBG_TRACE("return cir_p || buff_p NULL");
        return -1;
    }
    wr_p = &cir_p->data[cir_p->wr_pos];

    if (len > MAX_MEM_BLOCK_LEN || len + cir_p->valid_bytes > MAX_MEM_BLOCK_LEN)
    {
        DBG_TRACE("return not enough space for writing.");
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
    DBG_TRACE("valid_bytes:%d, len:%d, [wr:%d, rd:%d]"
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
        DBG_TRACE("return cir_p || buff_p NULL");
        return -1;
    }
    rd_p = &cir_p->data[cir_p->rd_pos];

    if (cir_p->valid_bytes == 0)
    {
        DBG_TRACE("return no data for reading");
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
    DBG_TRACE("valid_bytes:%d, read_len:%d, [wr:%d, rd:%d]"
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
        DBG_TRACE("return cir_p || buff_p NULL");
        return -1;
    }

    if (cir_p->valid_bytes == 0)
    {
        DBG_TRACE("return no data for reading");
        return -2;
    }
    bytes_to_search = cir_p->valid_bytes;
    i = cir_p->rd_pos;
    DBG_TRACE("rd_pos:%d, bytes_to_search:%d", i, bytes_to_search);
    
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

    DBG_TRACE("complete_line:%d, vbytes:%d, bytes_to_search:%d"
        , complete_line, cir_p->valid_bytes, bytes_to_search);
    
    if (!complete_line)
    {
        return -3;
    }
    else if (cir_p->valid_bytes - bytes_to_search > size)
    {
        DBG_TRACE("return not enough space for a line");
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
static void ppkg_queue_init(cfg_list_queue *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE("return queue_p NULL");
        return;
    }

    queue_p->qhead = queue_p->qtail = NULL;
    queue_p->len = 0;
}

/************************************************************************************
* Function: ppkg_queue_push
* Author @ Date: John.Wang@20200216
* Input:
* Return: length of queue
* Description: push node to the tail of queue
*************************************************************************************/
static int ppkg_queue_push(cfg_list_queue *queue_p, cmd_node_struct *node_p)
{   
    if (NULL == queue_p || NULL == node_p)
    {
        DBG_TRACE("return queue_p || node_p NULL");
        return -1;
    }

    if (NULL == queue_p->qtail)
    {
        /* Push the first node to queue */
        queue_p->qhead = queue_p->qtail = node_p;
        queue_p->qtail->next = NULL;
        queue_p->qtail->pre = NULL;
    }
    else
    {
        queue_p->qtail->next = node_p;
        node_p->pre = queue_p->qtail;
        node_p->next = NULL;
        queue_p->qtail = queue_p->qtail->next;
    }  

    queue_p->len++;
    DBG_TRACE("queue_p->len:%d", queue_p->len);

    return queue_p->len;
}

/************************************************************************************
* Function: ppkg_queue_pop
* Author @ Date: John.Wang@20200216
* Input:
* Return: length of queue
* Description: pop node from queue head
*************************************************************************************/
static int ppkg_queue_pop(cfg_list_queue *queue_p, cmd_node_struct **node_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE("return queue_p NULL");
        return -1;
    }

    if (NULL != queue_p->qhead)
    {
        *node_p = queue_p->qhead;
        if (queue_p->len > 1)
        {
            queue_p->qhead = queue_p->qhead->next;
            queue_p->qhead->pre = NULL;
            (*node_p)->next = NULL;  
        }              
    }

    queue_p->len--;
    DBG_TRACE("queue_p->len:%d, *node_p:%p", queue_p->len, *node_p);

    return queue_p->len;
}

/************************************************************************************
* Function: ppkg_queue_get_head
* Author @ Date: John.Wang@20200216
* Input:
* Return: pointer to queue head
* Description: get queue head but not pop
*************************************************************************************/
static cmd_node_struct* ppkg_queue_get_head(cfg_list_queue *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE("return queue_p NULL");
        return NULL;
    }

    return queue_p->qhead;
}

/************************************************************************************
* Function: ppkg_queue_get_len
* Author @ Date: John.Wang@20200219
* Input:
* Return: 
* Description: get the number of rest node in queue
*************************************************************************************/
static int ppkg_queue_get_len(cfg_list_queue *queue_p)
{
    if (NULL == queue_p)
    {
        DBG_TRACE("return queue_p NULL");
        return NULL;
    }

    return queue_p->len;
}

/************************************************************************************
* Function: ppkg_get_cfg_info
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: parse config file and get config information: path of param file, 
*              firmware version, etc...
*************************************************************************************/
static int ppkg_get_cfg_info(cfg_info_struct *cfg_info_p)
{
    if (NULL == cfg_info_p)
    {
        DBG_TRACE("return cfg_info_p NULL");
        return -1;
    }

    memset(cfg_info_p, 0, sizeof(cfg_info_struct));
    
    strcpy(cfg_info_p->fm_version, "GV300NR00A04V07M128_MIX_DET");
    strcpy(cfg_info_p->path_def_cfg, "D:\\forfun\\paramPackage_test\\A04V07_default.gv300");
    strcpy(cfg_info_p->path_cust_cfg, "D:\\forfun\\paramPackage_test\\C_GV300_A04V07_M0_0.gv300");
    strcpy(cfg_info_p->path_cust_ini, "D:\\forfun\\paramPackage_test\\C_GV300_A04V07_M0_0_at.ini");

    DBG_TRACE("cfg_info_p->fm_version:%s", cfg_info_p->fm_version);
    
    return 0;
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
            DBG_TRACE("circal buffer is full");
            ret = -3;
            break;
        }

        read_len = fread(temp_buff, 1, read_len, fp);
        DBG_TRACE("read_len:%d", read_len);
        if (MAX_TEMP_BLOCK_LEN == read_len)
        {
            ppkg_write_circal_buff(cir_buf, temp_buff, MAX_TEMP_BLOCK_LEN);
            memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
        }
        else
        {
            ppkg_write_circal_buff(cir_buf, temp_buff, read_len);
            ret = -3;
            DBG_TRACE("File read complete");
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
static int ppkg_get_cmd_type(const char *cmd_str, char *cmd_type)
{
    memcpy(cmd_type, (void*)(cmd_str + CMD_FRAME_HEAD_LEN + 1 + CMD_PREFIX_LEN), MAX_LEN_CMD_TYPE);
    return (CMD_FRAME_HEAD_LEN + 1 + CMD_PREFIX_LEN);
}

/************************************************************************************
* Function: ppkg_covert_to_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static line_type_enum ppkg_analysis_buff_line(const char *cmd_str, int cmd_len)
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
        line_type = CMD_LINE;
    }
    else
    {
    }

    DBG_TRACE("line_type:%d", line_type);
    
    return line_type;
}

/************************************************************************************
* Function: ppkg_covert_to_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: 
* Description: 
*************************************************************************************/
static int ppkg_covert_to_list(circal_buffer *cir_buf, cfg_list_queue *queue_p)
{
    cmd_node_struct *node_p = NULL;
    line_type_enum line_type = UNKOWN_LINE;
    int parsed_len = 0;
    int ret = 0;

    if (NULL == cir_buf || NULL == queue_p)
    {
        DBG_TRACE("return path, queue_p NULL");
        return -1;
    }
    
    memset(temp_buff, 0, MAX_TEMP_BLOCK_LEN);
    ret = ppkg_read_circal_buff_line(cir_buf, temp_buff, MAX_TEMP_BLOCK_LEN);
    if ((line_type = ppkg_analysis_buff_line(temp_buff, ret)) != UNKOWN_LINE)
    {
        if (CMD_LINE == line_type)
        {
            if ((node_p = (cmd_node_struct *)malloc(sizeof(cmd_node_struct) + ret)) == NULL)
            {
                DBG_TRACE("malloc failed");
                return -1;
            }
            memset(node_p, 0, (sizeof(cmd_node_struct) + ret));
            parsed_len = ppkg_get_cmd_type(temp_buff, node_p->cmd_type);

            /* Skip command type and right " */
            parsed_len += 4;
            node_p->cmd_len = ret - parsed_len;
            
            memcpy(node_p->cmd_str, &temp_buff[parsed_len], node_p->cmd_len);
            //DBG_TRACE("->cmd_type:%s, %d, %s", node_p->cmd_type, node_p->cmd_len, node_p->cmd_str);
            ppkg_queue_push(queue_p, node_p);
        }
        else if (VER_LINE == line_type)
        {
            /* Maybe used for comfire the version */
        }
        
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
static int ppkg_build_cmd_list(char* path, cfg_list_queue *queue_p)
{
    FILE *read_fp;
    int ret = 0;
    int cvt_ret = 0;
    #if 0
    int last_package = 0;
    int rd_pos = 0;
    int wr_pos = 0;
    #endif
    
    if (NULL == path || NULL == queue_p)
    {
        DBG_TRACE("return path, queue_p NULL");
        return -1;
    }

    if ((read_fp = fopen(path, B_FILE_READ_ONLY)) == NULL)
	{
		return -2;
	}
	else
	{
		DBG_TRACE("path=%s, read_fp=%x \n", path, read_fp);
	}

    fseek(read_fp, 0, SEEK_SET);
    do
    {
        if (ppkg_covert_to_list(&cir_buff, queue_p) < 0 && -3 != ret)
        {
            ret = ppkg_read_file(read_fp, &cir_buff);
        }
    }
    while (ppkg_circal_buff_get_valid_bytes(&cir_buff));
    
    DBG_TRACE("data convertion complete, rest bytes:%d"
                    , ppkg_circal_buff_get_valid_bytes(&cir_buff));
    ppkg_reset_circal_buff(&cir_buff);
    #if 0
    do 
    {
        if (wr_pos + MAX_MEM_BLOCK_LEN > MAX_MEM_BLOCK_LEN)
        {
            if (0 == rd_pos)
            {
                memset(&memory_block[0], 0, MAX_MEM_BLOCK_LEN);
                wr_pos = rd_pos;
                DBG_TRACE("shouldn't be here!");
                continue;
            }
            
            wr_pos -= rd_pos;
            if (wr_pos > 0)
            {
                memmove(&memory_block[0], &memory_block[rd_pos], wr_pos);
            }
            rd_pos = 0;
            memset(&memory_block[wr_pos], 0, MAX_MEM_BLOCK_LEN - wr_pos);
            DBG_TRACE("memmove, rest space %d bytes", MAX_MEM_BLOCK_LEN - wr_pos);
        }
        
        if (1 != fread(&memory_block[wr_pos], MAX_MEM_BLOCK_LEN - wr_pos, 1, read_fp))
        {
            last_package = 1;
        }
        else
        {
            wr_pos = MAX_MEM_BLOCK_LEN;
        }

        rd_pos += ppkg_covert_to_list(&memory_block[rd_pos], queue_p, last_package);
    }
    while(0 == last_package);
	#endif
	fclose(read_fp);
    
    return ret;
}

/************************************************************************************
* Function: ppkg_get_cmd_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: build a command list for compare between default configuration and 
*              customer configuration, then find a diff command list.
*************************************************************************************/
static void ppkg_cmd_list_init(cmd_list_struct *list_p)
{
    if (NULL == list_p)
    {
        DBG_TRACE("return list_p NULL");
        return;
    }

    ppkg_queue_init(&list_p->def_cfg);
    ppkg_queue_init(&list_p->cust_cfg);
    ppkg_queue_init(&list_p->cust_ini);
}

/************************************************************************************
* Function: ppkg_build_cmd_list
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: compare between default configuration and customer configuration, then 
*              generate a diff command list.
*************************************************************************************/
static int ppkg_compare_cmd_list(
            cfg_list_queue *cust_q, cfg_list_queue *def_q, cfg_list_queue *diff_q)
{
    cmd_node_struct *cust_node = NULL;
    cmd_node_struct *def_node = NULL;
    cmd_node_struct *diff_node = NULL;

    DBG_TRACE("ppkg_compare_cmd_list");
    
    if (NULL == cust_q || NULL == def_q || NULL == diff_q)
    {
        DBG_TRACE("return cust_q || def_q || diff_q NULL");
        return -1;
    }
    
    do
    {
        def_node = ppkg_queue_get_head(def_q); 
        ppkg_queue_pop(cust_q, &cust_node);
        if (NULL == def_node || NULL == cust_node)
        {
            DBG_TRACE("def_node:%p, cust_node:%p", def_node, cust_node);
            break;
        }
        
        do
        {
            if (0 == strcmp(cust_node->cmd_type, def_node->cmd_type) &&
                0 != strcmp(cust_node->cmd_str, def_node->cmd_str)
                )
            {
                if ((diff_node = (cmd_node_struct *)malloc(sizeof(cmd_node_struct))) == NULL)
                {
                    DBG_TRACE("malloc failed");
                    return -1;
                }
                memset(diff_node, 0, (sizeof(cmd_node_struct)));
                strcpy(diff_node->cmd_type, cust_node->cmd_type);
                DBG_TRACE("->diff_node:%s", diff_node->cmd_type);
               
                ppkg_queue_push(diff_q, diff_node);
                break;
            }
            else
            {
                def_node = def_node->next;
            }
        }
        while (def_node != NULL);        

        DBG_TRACE("cust_node:%p, custq_len:%d", cust_node, ppkg_queue_get_len(cust_q));
        free(cust_node);
    }
    while (ppkg_queue_get_len(cust_q));

    DBG_TRACE("diff_q->len:%d", diff_q->len);
    
    return diff_q->len;
}


/************************************************************************************
* Function: ppkg_gen
* Author @ Date: John.Wang@20200216
* Input:
* Return: parser result
* Description: pre-parser of the config file, parameter file
*************************************************************************************/
int ppkg_gen(void)
{
    ppkg_init_circal_buff(&cir_buff, memory_block);
    ppkg_cmd_list_init(&cmd_list);
    
    if (ppkg_get_cfg_info(&cfg_info) < 0)
    {
        DBG_TRACE("return ppkg_get_cfg_info");
        return -1;
    }
    //DBG_TRACE("------------>end");
    //return 0;
    if (ppkg_build_cmd_list(cfg_info.path_def_cfg, &cmd_list.def_cfg) < 0 &&
        ppkg_build_cmd_list(cfg_info.path_cust_cfg, &cmd_list.cust_cfg) < 0)
    {
        DBG_TRACE("cmd_list.def_cfg && cmd_list.cust_cfg");
    }

    /* compare between default configuration and customer configuration, then 
     * generate a diff command list.
     */
    if (ppkg_compare_cmd_list(&cmd_list.cust_cfg, &cmd_list.def_cfg, &cmd_list.diff_cfg) < 0)
    {
        DBG_TRACE("return ppkg_compare_cmd_list");
        return -3;
    }
#if 0
    if (ppkg_build_cmd_list(cfg_info.path_cust_ini, &cmd_list.cust_ini) >= 0)
    {
        //TODO: write AT file
    }
    else
    {
        DBG_TRACE("return ppkg_build_cmd_list");
        return -4;
    }
#endif
    return 0;
}

