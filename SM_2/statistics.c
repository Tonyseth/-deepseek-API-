#include "statistics.h"
#include <stdlib.h>

static void clearScreen() {
    system("cls");
}

void showStatistics(WordSystem* system) {
    clearScreen();
    printf("\n=== 学习统计 ===\n");
    printf("总单词数: %d\n", system->word_count);
    
    int mastered = 0;
    int in_progress = 0;
    int not_started = 0;
    int total_errors = 0;
    
    for (int i = 0; i < system->word_count; i++) {
        if (system->words[i].card.repetition >= 3) {
            mastered++;
        } else if (system->words[i].card.repetition > 0) {
            in_progress++;
        } else {
            not_started++;
        }
        total_errors += system->words[i].error_count;
    }
    
    printf("已掌握单词数(复习次数>=3): %d\n", mastered);
    printf("正在学习单词数: %d\n", in_progress);
    printf("未开始学习单词数: %d\n", not_started);
    
    printf("\n按Enter返回主菜单...");
    getchar();
    getchar();
} 