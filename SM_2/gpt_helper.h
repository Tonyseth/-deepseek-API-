#ifndef GPT_HELPER_H
#define GPT_HELPER_H

#include <curl/curl.h>

// 初始化GPT系统
void initGPTSystem(void);

// 清理GPT系统
void cleanupGPTSystem(void);

// 设置系统提示词
void setSystemPrompt(const char* prompt);

// 向GPT发送问题并获取回答
void askGPTHelper(const char* question);

// 向GPT发送JSON格式的问题并获取回答
void askGPTHelperJSON(const char* userAnswer, const char* correctAnswer, const char* question);

// 清除聊天历史
void clearChatHistory(void);

// 打印聊天历史
void printChatHistory(void);

#endif // GPT_HELPER_H 

