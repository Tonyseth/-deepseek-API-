#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "word.h"
#include "learning.h"
#include "statistics.h"
#include "ai_chat.h"
#include "sentence_practice.h"
#include "ascii_printer.h"

static void clearScreen() {
    system("cls");
}

static void printMenu() {
    printf("\n=== 专业英语学习程序 ===\n");
    printf("1. 开始单词学习\n");
    printf("2. 查看统计信息\n");
    printf("3. 开始句子练习\n");
    printf("4. 开始AI对话\n");
    printf("5. 退出程序\n");
    printf("请选择: ");
}

int main() {
    srand((unsigned int)time(NULL));
    WordSystem system;
    initWordSystem(&system);
    
    // 加载单词库
    if (loadWordsFromFile(&system, "words.txt") == 0) {
        printf("加载单词库失败！\n");
        return 1;
    }

    int choice;
    do {
        clearScreen();
        print_ascii_art("logo.txt");
        while (getchar() != '\n');
        clearScreen();
        printMenu();
		
        scanf_s("%d", &choice);

        switch (choice) {
            case 1:
                startLearning(&system);
                break;
            case 2:
                showStatistics(&system);
                break;
            case 3:
                startSentencePractice();
                break;
            case 4:
                startAIChat();
                break;
            case 5:
                print_ascii_art("logo.txt");
                while (getchar() != '\n');
                break;
            default:
                printf("\n无效选择，请重试！\n");
                break;
        }
    } while (choice != 5);

    // 清理资源
    cleanupAIChat();

    return 0;
} 