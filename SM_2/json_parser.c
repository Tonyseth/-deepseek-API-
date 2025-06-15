#include "json_parser.h"
#include "tonyJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_MESSAGE_LENGTH 4096
#define MAX_SCORE_LENGTH 10
#define MAX_SUGGESTION_LENGTH 1024


// 添加一个辅助函数来查找 JSON 对象中的字段
static int find_json_field(const char* json_str, const char* field_name, char** field_value) {
    char* field_start = strstr(json_str, field_name);
    if (!field_start) return 0;
    
    // 确保这是一个完整的字段名（前后有引号和冒号）
    char* colon = strchr(field_start + strlen(field_name), ':');
    if (!colon) return 0;
    
    // 找到字段值的开始位置
    char* value_start = colon + 1;
    
    // 跳过空白字符
    while (*value_start == ' ' || *value_start == '\t' || *value_start == '\n' || *value_start == '\r') {
        value_start++;
    }
    
    // 如果值是一个对象或数组，找到对应的结束位置
    if (*value_start == '{' || *value_start == '[') {
        int count = 1;
        char start_char = *value_start;
        char end_char = (start_char == '{') ? '}' : ']';
        char* p = value_start + 1;
        
        while (*p && count > 0) {
            if (*p == start_char) count++;
            else if (*p == end_char) count--;
            p++;
        }
    }
    
    *field_value = value_start;
    return 1;
}

// 添加一个函数来解析转义的 JSON 字符串
static char* unescape_json_string(const char* input, size_t len) {
    char* output = (char*)malloc(len + 1);
    if (!output) return NULL;
    
    size_t i = 0, j = 0;
    while (i < len) {
        if (input[i] == '\\' && i + 1 < len) {
            switch (input[i + 1]) {
                case 'n': output[j++] = '\n'; i++; break;
                case 'r': output[j++] = '\r'; i++; break;
                case 't': output[j++] = '\t'; i++; break;
                case '\\': output[j++] = '\\'; i++; break;
                case '\"': output[j++] = '\"'; i++; break;
                default: output[j++] = input[i];
            }
        } else {
            output[j++] = input[i];
        }
        i++;
    }
    output[j] = '\0';
    return output;
}

// 添加一个辅助函数来提取字段值
static int extract_field_value(char* value_start, char* output, size_t max_len) {
    // 跳过空白字符
    while (*value_start == ' ' || *value_start == '\t' || *value_start == '\n' || *value_start == '\r') {
        value_start++;
    }
    
    // 检查值是否是数字
    if (isdigit(*value_start)) {
        // 找到数字的结束位置
        char* value_end = value_start;
        while (isdigit(*value_end)) {
            value_end++;
        }
        
        // 计算值的长度并复制
        size_t value_len = value_end - value_start;
        if (value_len >= max_len) {
            value_len = max_len - 1;
        }
        
        memcpy(output, value_start, value_len);
        output[value_len] = '\0';
        
        return 1;
    }
    
    // 如果不是数字，则必须是字符串（以引号开始）
    if (*value_start != '\"') {
        return 0;
    }
    
    value_start++; // 跳过开头的引号
    
    // 找到值的结束位置（考虑转义的引号）
    char* value_end = value_start;
    while (*value_end) {
        if (*value_end == '\"' && (value_end == value_start || *(value_end-1) != '\\')) {
            break;
        }
        value_end++;
    }
    
    if (!*value_end) {
        return 0;
    }
    
    // 计算值的长度并复制
    size_t value_len = value_end - value_start;
    if (value_len >= max_len) {
        value_len = max_len - 1;
    }
    
    memcpy(output, value_start, value_len);
    output[value_len] = '\0';
    
    return 1;
}

int parse_deepseek_response(const char* response, char* score, char* suggestions) {
    // 查找 choices 数组
    char* choices_value;
    if (!find_json_field(response, "\"choices\"", &choices_value)) {
        return 0;
    }
    
    // 查找第一个 message 对象
    char* message_value;
    if (!find_json_field(choices_value, "\"message\"", &message_value)) {
        return 0;
    }
    
    // 查找 content 字段
    char* content_value;
    if (!find_json_field(message_value, "\"content\"", &content_value)) {
        return 0;
    }
    
    // 提取 content 的值（去除引号）
    if (*content_value != '\"') {
        return 0;
    }
    content_value++; // 跳过开头的引号
    
    // 找到 content 值的结束位置
    char* content_end = strrchr(content_value, '\"');
    if (!content_end) {
        return 0;
    }
    
    // 解析转义的 JSON 字符串
    char* unescaped_content = unescape_json_string(content_value, content_end - content_value);
    if (!unescaped_content) return 0;
    
    // 查找 score 和 suggestions
    char* score_value;
    char* suggestions_value;
    
    if (find_json_field(unescaped_content, "\"score\"", &score_value) &&
        find_json_field(unescaped_content, "\"suggestions\"", &suggestions_value)) {
        
        // 提取 score
        if (!extract_field_value(score_value, score, MAX_SCORE_LENGTH)) {
            free(unescaped_content);
            return 0;
        }
        
        // 提取 suggestions
        if (!extract_field_value(suggestions_value, suggestions, MAX_SUGGESTION_LENGTH)) {
            free(unescaped_content);
            return 0;
        }
        
        free(unescaped_content);
        return 1;
    }
    
    free(unescaped_content);
    return 0;
} 