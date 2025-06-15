#include "tonyJSON.h"
#include <stdio.h>
#include <string.h>
tonyJSON_DECODE_ITEM * tonyJSON_DECODE_ITEM_Create(int maxSize)
{
    if(maxSize <= 0 || (size_t)maxSize > SIZE_MAX/sizeof(int)) return NULL;
    
    tonyJSON_DECODE_ITEM * dc_item_ptr = (tonyJSON_DECODE_ITEM*)malloc(sizeof(tonyJSON_DECODE_ITEM));
    if(!dc_item_ptr) return NULL;
    
    dc_item_ptr->count = 0;
    dc_item_ptr->index = (int*)malloc((size_t)maxSize * sizeof(int));
    if(!dc_item_ptr->index) {
        free(dc_item_ptr);
        return NULL;
    }
    return dc_item_ptr;
}
tonyJSON_ITEM_SOLO * tonyJSON_ITEM_SOLO_Create(int maxSize)
{
    if(maxSize <= 0) return NULL;
    
    tonyJSON_ITEM_SOLO * dc_item_solo_ptr = (tonyJSON_ITEM_SOLO*)malloc(maxSize * sizeof(tonyJSON_ITEM_SOLO));
    if(!dc_item_solo_ptr) return NULL;
    
    for(int i=0; i<maxSize; i++) {
        dc_item_solo_ptr[i].index_e = 0;
        dc_item_solo_ptr[i].index_s = 0;
    }
    return dc_item_solo_ptr;
}
void tonyJSON_ITEM_SOLO_Free(tonyJSON_ITEM_SOLO * dc_item_solo_ptr)
{
    free(dc_item_solo_ptr);
}
void tonyJSON_DECODE_ITEM_Free(tonyJSON_DECODE_ITEM * dc_item_ptr)
{
    free(dc_item_ptr->index);
    free(dc_item_ptr);
}
void tonyJSON_decode_json_str(char * jsonStr)
{
    char * read_ptr = (char *)jsonStr;
    int search=1;

    // printf("len=%d\n",strlen(jsonStr));
    size_t len = strlen(jsonStr);
    tonyJSON_DECODE_ITEM * dc_colon   = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_brace_l = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_brace_r = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_comma   = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_ITEM_SOLO   * dc_item_solo_brace  = tonyJSON_ITEM_SOLO_Create(len);
    int dc_item_solo_brace_count=0;
    // tonyJSON_DECODE_ITEM * dc_colon = tonyJSON_DECODE_ITEM_Create(len);
    int gid=0;
    while(search)
    {

        if( (*read_ptr) == '\0' || (read_ptr == NULL))
        {
            search=0;
        }
        else
        {
            // printf("%c\n",*read_ptr);
            switch((*read_ptr))
            {
                case '{':
                {
                    dc_brace_l->index[dc_brace_l->count++]=gid;
                }break;
                case '}': 
                {
                    dc_brace_r->index[dc_brace_r->count++]=gid;
                    if(dc_brace_l->count>0)
                    {
                        if(dc_brace_l->index[dc_brace_l->count-1] < gid)
                        {
                            dc_item_solo_brace[dc_item_solo_brace_count].index_e = gid;
                            dc_item_solo_brace[dc_item_solo_brace_count++].index_s = dc_brace_l->index[dc_brace_l->count-1];
                            dc_brace_l->count--;
                        }
                    }
                }break;
                case ':':
                {

                }break;
            }
        }
        gid++;
        read_ptr++;
    }

    for(int j=0;j<dc_item_solo_brace_count;j++)
    {
        // tonyJSON_item_solo_print(dc_item_solo_brace+j,jsonStr,"dc_item_solo_brace");
        int start_index = (dc_item_solo_brace+j)->index_s;
        int end_index   = (dc_item_solo_brace+j)->index_e;
        int sub_len     = end_index - start_index+1;
        tonyJSON_DECODE_ITEM * sub_dc_colon   = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_brace_l = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_brace_r = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_comma   = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_dqm     = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_ITEM *total_items = (tonyJSON_ITEM*)malloc(sub_len * sizeof(tonyJSON_ITEM));
        int total_items_count=0;
        // printf("j=%d\n",j);
        int skip1=0,skip2=0;
        int start_counter_string=0;
        for(int k=(start_index+1);k<end_index;k++)
        {
            char search_char = jsonStr[k];
            int skip =  (skip1 & skip2);
            switch(search_char)
            {
                case '[':
                {
                    if(!skip1) skip1 = 1;
                }break;
                case ']':
                {
                    if(skip1) skip1 = 0;
                }break;
                case '{':
                {
                    if(!skip2) skip2 = 1;
                }break;
                case '}':
                {
                    if(skip2) skip2 = 0;
                }break;
                case '"':
                {
                    if(!skip) 
                    {
                        sub_dc_dqm->index[sub_dc_dqm->count++]      = k;
                    
                        start_counter_string++;
                        if(start_counter_string==-1 && total_items_count>=1)
                        {
                            total_items[total_items_count-1].value_index_s = k+1;
                        }
                        if(start_counter_string==0 && total_items_count>=1)
                        {
                            total_items[total_items_count-1].value_index_e = k-1;
                        }
                    }
                }break;
                case ':':
                {
                    if(!skip) 
                    {
                        sub_dc_colon->index[sub_dc_colon->count++]  = k;
                        if(sub_dc_dqm->count>2 && start_counter_string>=0)
                        {
                            total_items[total_items_count].label_index_s = sub_dc_dqm->index[sub_dc_dqm->count-2]+1;
                            total_items[total_items_count].label_index_e = k-2;//sub_dc_dqm->index[sub_dc_dqm->count-1]-1;
                            total_items[total_items_count].value_index_s = k+1;
                            total_items[total_items_count].value_index_e = end_index;
                            total_items_count++;
                            start_counter_string=-2;
                        }
                    }
                }break;
                case ',':
                {
                    if(!skip) 
                    {
                        sub_dc_comma->index[sub_dc_comma->count++]  = k;
                    }
                }break;
            }
        }
        //
        // printf("comma: %d\n",sub_dc_comma->count);
        // printf("dqm  : %d\n",sub_dc_dqm->count);
        // printf("colon: %d\n",sub_dc_colon->count);
        // printf("total_items_count: %d\n",total_items_count);
        for(int l=0; l<total_items_count; l++)
        {
            int label_lens = calculate_safe_length(total_items[l].label_index_s, total_items[l].label_index_e);
            if (label_lens <= 0) continue;

            int value_lens = calculate_safe_length(total_items[l].value_index_s, total_items[l].value_index_e);
            if (value_lens <= 0) continue;
            
            char *log_str = (char*)malloc(label_lens);
            char *log_str_v = (char*)malloc(value_lens);
            
            if(log_str == NULL || log_str_v == NULL) {
                if(log_str) free(log_str);
                if(log_str_v) free(log_str_v);
                continue;
            }
            
            memcpy(log_str, jsonStr + total_items[l].label_index_s, label_lens-1);
            log_str[label_lens-1] = '\0';
            memcpy(log_str_v, jsonStr + total_items[l].value_index_s, value_lens-1);
            log_str_v[value_lens-1] = '\0';
            
            log_label_value_string(log_str, log_str_v, value_lens-1);
            
            free(log_str);
            free(log_str_v);
        }
        tonyJSON_DECODE_ITEM_Free(sub_dc_colon);
        tonyJSON_DECODE_ITEM_Free(sub_dc_brace_l);
        tonyJSON_DECODE_ITEM_Free(sub_dc_brace_r);
        tonyJSON_DECODE_ITEM_Free(sub_dc_comma);
        tonyJSON_DECODE_ITEM_Free(sub_dc_dqm);
        
        //
        free(total_items);
    }

    tonyJSON_DECODE_ITEM_Free(dc_colon);
    tonyJSON_DECODE_ITEM_Free(dc_brace_l);
    tonyJSON_DECODE_ITEM_Free(dc_brace_r);
    tonyJSON_DECODE_ITEM_Free(dc_comma);
    tonyJSON_ITEM_SOLO_Free(dc_item_solo_brace);
}

void tonyJSON_item_solo_print(tonyJSON_ITEM_SOLO * item,char * jsonStr,char * tag)
{
    char *log_str = (char*)malloc(item->index_e - item->index_s + 2);
    memcpy(log_str, jsonStr + item->index_s, item->index_e - item->index_s + 1);
    log_str[item->index_e - item->index_s + 1] = '\0';
    printf("[%s]: %s\n", tag, log_str);
    free(log_str);
}
void log_label_value_string(char * label,char* value,int value_size)
{
            printf("[%s]: ",label);
            char * ptr = value;
            printf("%c",*ptr);
            for(int p=1;p<value_size;p++)
            {
                if(ptr[p-1]=='\\' && ptr[p]=='n')
                {
                    printf("\n");
                }
                else if(ptr[p]!='\\')
                {
                    printf("%c",ptr[p]);
                }
            }
            printf("\n");
        
}
void log_tonyJSON_item(tonyJSON_ITEM tonyItem,char * str)
{
    int label_lens = tonyItem.label_index_e - tonyItem.label_index_s +2;
    int value_lens = tonyItem.value_index_e - tonyItem.value_index_s +2;
    
    char *log_str = (char*)malloc(label_lens);
    char *log_str_v = (char*)malloc(value_lens);
    
    memcpy(log_str, str + tonyItem.label_index_s, label_lens - 1);
    log_str[label_lens - 1] = '\0';
    memcpy(log_str_v, str + tonyItem.value_index_s, value_lens - 1);
    log_str_v[value_lens - 1] = '\0';
    
    log_label_value_string(log_str, log_str_v, value_lens - 1);
    
    free(log_str);
    free(log_str_v);
}
int find_item_with_label_str(char * jsonStr, char * label_find, tonyJSON_ITEM * find_item_p)
{
    if (!jsonStr || !label_find || !find_item_p) return 0;

    int found = 0;
    tonyJSON_ITEM find_Item = {0}; // 初始化结构体
    char * read_ptr = (char *)jsonStr;
    int search = 1;

    // printf("len=%d\n",strlen(jsonStr));
    size_t len = strlen(jsonStr);
    tonyJSON_DECODE_ITEM * dc_colon   = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_brace_l = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_brace_r = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_DECODE_ITEM * dc_comma   = tonyJSON_DECODE_ITEM_Create(len);
    tonyJSON_ITEM_SOLO   * dc_item_solo_brace  = tonyJSON_ITEM_SOLO_Create(len);
    int dc_item_solo_brace_count=0;
    // tonyJSON_DECODE_ITEM * dc_colon = tonyJSON_DECODE_ITEM_Create(len);
    int gid=0;
    while(search)
    {

        if( (*read_ptr) == '\0' || (read_ptr == NULL))
        {
            search=0;
        }
        else
        {
            // printf("%c\n",*read_ptr);
            switch((*read_ptr))
            {
                case '{':
                {
                    dc_brace_l->index[dc_brace_l->count++]=gid;
                }break;
                case '}': 
                {
                    dc_brace_r->index[dc_brace_r->count++]=gid;
                    if(dc_brace_l->count>0)
                    {
                        if(dc_brace_l->index[dc_brace_l->count-1] < gid)
                        {
                            dc_item_solo_brace[dc_item_solo_brace_count].index_e = gid;
                            dc_item_solo_brace[dc_item_solo_brace_count++].index_s = dc_brace_l->index[dc_brace_l->count-1];
                            dc_brace_l->count--;
                        }
                    }
                }break;
                case ':':
                {

                }break;
            }
        }
        gid++;
        read_ptr++;
    }

    for(int j=0; j<dc_item_solo_brace_count; j++)
    {
        // tonyJSON_item_solo_print(dc_item_solo_brace+j,jsonStr,"dc_item_solo_brace");
        int start_index = (dc_item_solo_brace+j)->index_s;
        int end_index   = (dc_item_solo_brace+j)->index_e;
        int sub_len     = end_index - start_index+1;
        tonyJSON_DECODE_ITEM * sub_dc_colon   = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_brace_l = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_brace_r = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_comma   = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_DECODE_ITEM * sub_dc_dqm     = tonyJSON_DECODE_ITEM_Create(sub_len);
        tonyJSON_ITEM *total_items = (tonyJSON_ITEM*)malloc(sub_len * sizeof(tonyJSON_ITEM));
        int total_items_count=0;
        // printf("j=%d\n",j);
        int skip1=0,skip2=0;
        int start_counter_string=0;
        for(int k=(start_index+1);k<end_index;k++)
        {
            char search_char = jsonStr[k];
            int skip =  (skip1 & skip2);
            switch(search_char)
            {
                case '[':
                {
                    if(!skip1) skip1 = 1;
                }break;
                case ']':
                {
                    if(skip1) skip1 = 0;
                }break;
                case '{':
                {
                    if(!skip2) skip2 = 1;
                }break;
                case '}':
                {
                    if(skip2) skip2 = 0;
                }break;
                case '"':
                {
                    if(!skip) 
                    {
                        sub_dc_dqm->index[sub_dc_dqm->count++]      = k;
                    
                        start_counter_string++;
                        if(start_counter_string==-1 && total_items_count>=1)
                        {
                            total_items[total_items_count-1].value_index_s = k+1;
                        }
                        if(start_counter_string==0 && total_items_count>=1)
                        {
                            total_items[total_items_count-1].value_index_e = k-1;
                        }
                    }
                }break;
                case ':':
                {
                    if(!skip) 
                    {
                        sub_dc_colon->index[sub_dc_colon->count++]  = k;
                        if(sub_dc_dqm->count>2 && start_counter_string>=0)
                        {
                            total_items[total_items_count].label_index_s = sub_dc_dqm->index[sub_dc_dqm->count-2]+1;
                            total_items[total_items_count].label_index_e = k-2;//sub_dc_dqm->index[sub_dc_dqm->count-1]-1;
                            total_items[total_items_count].value_index_s = k+1;
                            total_items[total_items_count].value_index_e = end_index;
                            total_items_count++;
                            start_counter_string=-2;
                        }
                    }
                }break;
                case ',':
                {
                    if(!skip) 
                    {
                        sub_dc_comma->index[sub_dc_comma->count++]  = k;
                    }
                }break;
            }
        }
        //
        // printf("comma: %d\n",sub_dc_comma->count);
        // printf("dqm  : %d\n",sub_dc_dqm->count);
        // printf("colon: %d\n",sub_dc_colon->count);
        // printf("total_items_count: %d\n",total_items_count);
        for(int l=0; l<total_items_count; l++)
        {
            int label_lens = calculate_safe_length(total_items[l].label_index_s, total_items[l].label_index_e);
            if (label_lens <= 0) continue;

            int value_lens = calculate_safe_length(total_items[l].value_index_s, total_items[l].value_index_e);
            if (value_lens <= 0) continue;
            
            char *log_str = (char*)malloc(label_lens);
            if (!log_str) continue;
            
            memcpy(log_str, jsonStr + total_items[l].label_index_s, label_lens-1);
            log_str[label_lens-1] = '\0';
            
            if(strcmp(label_find, log_str) == 0)
            {
                found = 1;
                find_Item = total_items[l];
            }
            
            free(log_str);
        }
        tonyJSON_DECODE_ITEM_Free(sub_dc_colon);
        tonyJSON_DECODE_ITEM_Free(sub_dc_brace_l);
        tonyJSON_DECODE_ITEM_Free(sub_dc_brace_r);
        tonyJSON_DECODE_ITEM_Free(sub_dc_comma);
        tonyJSON_DECODE_ITEM_Free(sub_dc_dqm);
        
        //
        free(total_items);
    }

    tonyJSON_DECODE_ITEM_Free(dc_colon);
    tonyJSON_DECODE_ITEM_Free(dc_brace_l);
    tonyJSON_DECODE_ITEM_Free(dc_brace_r);
    tonyJSON_DECODE_ITEM_Free(dc_comma);
    tonyJSON_ITEM_SOLO_Free(dc_item_solo_brace);
    if(found) {
        *find_item_p = find_Item;
    }
    return found;
}

int calculate_safe_length(int start, int end) {
    if (end < start) return 0;
    long long diff = (long long)end - (long long)start + 2;
    if (diff > INT_MAX) return 0;
    return (int)diff;
}