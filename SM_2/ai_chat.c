#include "ai_chat.h"
#include "gpt_helper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ascii_printer.h"
// 系统提示词
static const char* SYSTEM_PROMPT =
"你是一个英语老师，你的主要职责是帮助用户学习和掌握英语词汇和表达。";



void initAIChat(void) {
    // 初始化AI聊天相关资源
    initGPTSystem();
    
    // 设置系统提示词
    setSystemPrompt(SYSTEM_PROMPT);
    
    // 清除历史记录
    clearChatHistory();
}

void startAIChat(void) {
    system("cls");

    printf("\n=== AI助手 ===\n");
    printf("请输入你的问题（或输入'quit'返回，'history'查看历史记录）：\n");
    
    while (getchar() != '\n');

    char input[1024];
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0) break;
        if (strcmp(input, "history") == 0) {
            printChatHistory();
            continue;
        }
        if (strlen(input) > 0) {
            askGPTHelper(input);
        }
    }
}

void cleanupAIChat(void) {
    cleanupGPTSystem();
} 