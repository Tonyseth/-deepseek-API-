#include "gpt_helper.h"
#include "tonyJSON.h"
#include "json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define MAX_MESSAGES 50
#define MAX_MESSAGE_LENGTH 4096
#define MAX_SCORE_LENGTH 10
#define MAX_SUGGESTION_LENGTH 1024

typedef struct {
    char role[20];
    char content[MAX_MESSAGE_LENGTH];
} ChatMessage;

typedef struct {
    ChatMessage messages[MAX_MESSAGES];
    int count;
} ChatHistory;

static ChatHistory chat_history = {0};

static char* openai_key = "sk-1c567cf75e684c6f9dd3db8943a2cc0e";
static char global_response[1024 * 128];
static int  global_response_offset = 0;

static char buffer[4096] = { 0 }; // 添加缓冲区
static size_t buffer_pos = 0;

static char current_chunk[MAX_MESSAGE_LENGTH] = {0};
static int chunk_offset = 0;

static char json_response[MAX_MESSAGE_LENGTH] = {0};
static int json_response_offset = 0;

static char system_prompt[4096] = {0};

static void buildRequestJSON(const char* user_input, char* output, size_t output_size);
static void json_escape_string(const char* input, char* output, size_t output_size);
static int progress_callback(void *clientp,
                           curl_off_t dltotal, curl_off_t dlnow,
                           curl_off_t ultotal, curl_off_t ulnow);

static size_t write_callback_chat(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t response_size = size * nmemb;
    char* response = (char*)malloc(response_size + 1);
    if (!response) return 0;

    memcpy(response, ptr, response_size);
    response[response_size] = '\0';

    // 处理流式数据
    char* line, * next_token = NULL;
    char* temp = _strdup(response);  // 使用 _strdup 替代 strdup
    if (!temp) {
        free(response);
        return 0;
    }

    line = strtok_s(temp, "\n", &next_token);  // 使用 strtok_s 替代 strtok_r

    while (line) {
        if (strstr(line, "[DONE]")) {
            // 处理结束标记
            if (chunk_offset > 0) {
                if (chat_history.count < MAX_MESSAGES) {
                    strncpy_s(chat_history.messages[chat_history.count].role,
                        sizeof(chat_history.messages[chat_history.count].role),
                        "assistant", _TRUNCATE);
                    strncpy_s(chat_history.messages[chat_history.count].content,
                        sizeof(chat_history.messages[chat_history.count].content),
                        current_chunk, _TRUNCATE);
                    chat_history.count++;
                }
            }
        }
        else if (strstr(line, "content")) {
            // 解析content字段
            tonyJSON_ITEM chatFromGPT;
            if (find_item_with_label_str(line, (char*)"content", &chatFromGPT)) {
                char* content_start = line + chatFromGPT.value_index_s;
                char* content_end = strchr(content_start, '\"');
                if (content_end) {
                    size_t content_length = content_end - content_start;
                    char* content = (char*)malloc(content_length + 1);
                    if (content) {
                        memcpy(content, content_start, content_length);
                        content[content_length] = '\0';

                        printf("%s", content);
                        fflush(stdout);

                        // 添加到当前块
                        if (chunk_offset + content_length < MAX_MESSAGE_LENGTH) {
                            memcpy(current_chunk + chunk_offset, content, content_length);
                            chunk_offset += content_length;
                            current_chunk[chunk_offset] = '\0';
                        }

                        free(content);
                    }
                }
            }
        }
        line = strtok_s(NULL, "\n", &next_token);  // 继续使用 strtok_s
    }

    free(temp);
    free(response);
    return response_size;
}

static size_t write_callback_json(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t response_size = size * nmemb;
    char* response = (char*)malloc(response_size + 1);
    if (!response) return 0;
    
    memcpy(response, ptr, response_size);
    response[response_size] = '\0';
    
    if ((size_t)json_response_offset + response_size < MAX_MESSAGE_LENGTH) {
        memcpy(json_response + json_response_offset, response, response_size);
        json_response_offset = (int)(json_response_offset + response_size);
        json_response[json_response_offset] = '\0';
    }
    
    free(response);
    return response_size;
}

static void send_request_chat(char* data)
{
    CURL* curl;
    CURLcode res;

    global_response_offset = 0;
    memset(global_response, 0, sizeof(global_response));

    curl = curl_easy_init();
    if (curl) {
        char* header_auth = (char*)malloc(1000);
        char* payload = (char*)malloc(16384);
        
        if (!header_auth || !payload) {
            if (header_auth) free(header_auth);
            if (payload) free(payload);
            fprintf(stderr, "内存分配失败\n");
            return;
        }
        
        sprintf_s(header_auth, 1000, "Authorization: Bearer %s", openai_key);
        
        buildRequestJSON(data, payload, 16384);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, header_auth);
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_chat);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "\nRequest failed: %s\n", curl_easy_strerror(res));
            if (chat_history.count > 0) {
                chat_history.count--;
            }
            
            memset(current_chunk, 0, sizeof(current_chunk));
            chunk_offset = 0;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(header_auth);
        free(payload);
        printf("\n");
    }
}

static int progress_callback(void *clientp,
                           curl_off_t dltotal, curl_off_t dlnow,
                           curl_off_t ultotal, curl_off_t ulnow)
{
    return 0;
}

void initGPTSystem(void) {
    curl_global_init(CURL_GLOBAL_ALL);
}

void cleanupGPTSystem(void) {
    curl_global_cleanup();
}

void askGPTHelper(const char* question) {
    if (!question || strlen(question) == 0) {
        return;
    }

    int prev_count = chat_history.count;

    if (chat_history.count < MAX_MESSAGES) {
        strncpy_s(chat_history.messages[chat_history.count].role,
                 sizeof(chat_history.messages[chat_history.count].role),
                 "user", _TRUNCATE);
        strncpy_s(chat_history.messages[chat_history.count].content,
                 sizeof(chat_history.messages[chat_history.count].content),
                 question, _TRUNCATE);
        chat_history.count++;
    }
    
    send_request_chat((char*)question);
    
    if (chat_history.count == prev_count + 1) {
        printf("\n按Enter继续...");
        while (getchar() != '\n');
    }
}

void clearChatHistory(void) {
    chat_history.count = 0;
}

void printChatHistory(void) {
    printf("\n=== Chat History ===\n");
    for (int i = 0; i < chat_history.count; i++) {
        printf("[%s]: %s\n", 
               chat_history.messages[i].role, 
               chat_history.messages[i].content);
    }
    printf("=================\n");
}

static void json_escape_string(const char* input, char* output, size_t output_size) {
    size_t i = 0, j = 0;
    
    output[j++] = '\"';  // 添加开头的引号
    
    while (input[i] != '\0' && j < output_size - 3) {  // 留出结尾引号和空字符空间
        if (input[i] == '\"' || input[i] == '\\' || input[i] == '\n' || input[i] == '\r') {
            output[j++] = '\\';
            if (input[i] == '\n') {
                output[j++] = 'n';
            } else if (input[i] == '\r') {
                output[j++] = 'r';
            } else {
                output[j++] = input[i];
            }
        } else {
            output[j++] = input[i];
        }
        i++;
    }
    
    output[j++] = '\"';  // 添加结尾的引号
    output[j] = '\0';
}

void askGPTHelperJSON(const char* user_english, const char* correct_english, const char* chinese) {
    memset(json_response, 0, sizeof(json_response));
    json_response_offset = 0;
    
    CURL* curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        char* header_auth = (char*)malloc(1000);
        char* system_prompt = (char*)malloc(2048);
        char* user_prompt = (char*)malloc(4096);
        char* escaped_system_prompt = (char*)malloc(4096);
        char* escaped_user_prompt = (char*)malloc(8192);
        char* messages = (char*)malloc(16384);
        char* payload = (char*)malloc(32768);
        
        struct curl_slist* headers = NULL;
        
        if (!header_auth || !system_prompt || !user_prompt || 
            !escaped_system_prompt || !escaped_user_prompt || 
            !messages || !payload) {
            if (header_auth) free(header_auth);
            if (system_prompt) free(system_prompt);
            if (user_prompt) free(user_prompt);
            if (escaped_system_prompt) free(escaped_system_prompt);
            if (escaped_user_prompt) free(escaped_user_prompt);
            if (messages) free(messages);
            if (payload) free(payload);
            if (headers) curl_slist_free_all(headers);
            fprintf(stderr, "内存分配失败\n");
            return;
        }
        
        sprintf_s(header_auth, 1000, "Authorization: Bearer %s", openai_key);
        
        // 系统提示
        snprintf(system_prompt, 2048, 
            "你是一个英语教师，负责评估学生的英语翻译。请按照以下JSON格式输出评估结果：\n"
            "{\n"
            "    \"score\": \"分数（1-100）\",\n"
            "    \"suggestions\": \"改进建议\"\n"
            "}");
        
        // 用户提示
        snprintf(user_prompt, 4096,
            "中文句子：%s\n"
            "标准英文：%s\n"
            "学生答案：%s\n"
            "请评估学生答案的准确性，给出分数和具体的改进建议。",
            chinese, correct_english, user_english);

        // 转义提示内容
        json_escape_string(system_prompt, escaped_system_prompt, 4096);
        json_escape_string(user_prompt, escaped_user_prompt, 8192);

        // 构建消息数组
        snprintf(messages, 16384,
            "[{\"role\":\"system\",\"content\":%s},{\"role\":\"user\",\"content\":%s}]",
            escaped_system_prompt, escaped_user_prompt);

        snprintf(payload, 32768,
            "{\"model\":\"deepseek-chat\",\"messages\":%s,\"response_format\":{\"type\":\"json_object\"}}",
            messages);

        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, header_auth);
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_json);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "\nRequest failed: %s\n", curl_easy_strerror(res));
        } else {
            char score[MAX_SCORE_LENGTH] = {0};
            char suggestions[MAX_SUGGESTION_LENGTH] = {0};
            
            if (parse_deepseek_response(json_response, score, suggestions)) {
                printf("\n评分结果：\n");
                printf("得分: %s\n", score);
                printf("----------------------------------------\n");
                printf("你的答案：%s\n", user_english);
                printf("标准答案：%s\n", correct_english);
                printf("----------------------------------------\n");
                printf("改进建议: %s\n", suggestions);
            } else {
                printf("\n解析响应失败！\n");
            }
        }

        // 清理资源
        curl_easy_cleanup(curl);
        if (headers) curl_slist_free_all(headers);
        free(header_auth);
        free(system_prompt);
        free(user_prompt);
        free(escaped_system_prompt);
        free(escaped_user_prompt);
        free(messages);
        free(payload);
    }
} 

void setSystemPrompt(const char* prompt) {
    strncpy_s(system_prompt, sizeof(system_prompt), prompt, _TRUNCATE);
}

static void buildRequestJSON(const char* user_input, char* output, size_t output_size) {
    char* escaped_system_prompt = (char*)malloc(8192);
    char* escaped_user_input = (char*)malloc(4096);
    
    if (!escaped_system_prompt || !escaped_user_input) {
        if (escaped_system_prompt) free(escaped_system_prompt);
        if (escaped_user_input) free(escaped_user_input);
        snprintf(output, output_size, "{\"error\":\"内存分配失败\"}");
        return;
    }
    
    json_escape_string(system_prompt, escaped_system_prompt, 8192);
    json_escape_string(user_input, escaped_user_input, 4096);
    
    snprintf(output, output_size,
        "{"
        "\"model\": \"deepseek-coder\","
        "\"messages\": ["
        "{"
        "\"role\": \"system\","
        "\"content\": %s"
        "},"
        "{"
        "\"role\": \"user\","
        "\"content\": %s"
        "}"
        "],"
        "\"stream\": true"
        "}", 
        escaped_system_prompt,
        escaped_user_input
    );
    
    free(escaped_system_prompt);
    free(escaped_user_input);
} 