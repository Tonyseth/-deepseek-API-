#ifndef __tony_JSON_H__
#define __tony_JSON_H__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
typedef struct tonyJSON_DECODE_ITEM_t
{
    int *index;
    int count;
}tonyJSON_DECODE_ITEM;
typedef struct tonyJSON_ITEM_SOLO_t
{
    int index_s;
    int index_e;
}tonyJSON_ITEM_SOLO;
typedef struct tonyJSON_ITEM_t
{
    int label_index_s;
    int label_index_e;
    int value_index_s;
    int value_index_e;
}tonyJSON_ITEM;
int calculate_safe_length(int start, int end);
void tonyJSON_decode_json_str(char * jsonStr);
tonyJSON_ITEM_SOLO * tonyJSON_ITEM_SOLO_Create(int maxSize);
void tonyJSON_ITEM_SOLO_Free(tonyJSON_ITEM_SOLO * dc_item_solo_ptr);
tonyJSON_DECODE_ITEM * tonyJSON_DECODE_ITEM_Create(int maxSize);
void tonyJSON_DECODE_ITEM_Free(tonyJSON_DECODE_ITEM * dc_item_ptr);
void tonyJSON_item_solo_print(tonyJSON_ITEM_SOLO * item,char * jsonStr,char * tag);
void log_label_value_string(char * label,char* value,int value_size);
void log_tonyJSON_item(tonyJSON_ITEM tonyItem,char * str);
int find_item_with_label_str(char * jsonStr,char * label_find,tonyJSON_ITEM * find_item_p);
#endif //__tony_JSON_H__