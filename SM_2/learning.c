#include "learning.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define MAX_INPUT 100

static void clearScreen() {
    system("cls");
}

void startLearning(WordSystem* system) {
    // 每次进入学习功能时增加session
    system->current_session++;
    
    char input[MAX_INPUT];
    Word* current_word;
    int words_studied = 0;  // 记录本次学习的单词数

    while ((current_word = getNextWord(system)) != NULL) {
        clearScreen();
        printf("\n中文释义: %s\n", current_word->meaning);
        printf("请输入对应的英文单词: ");
        
        scanf_s("%s", input, (unsigned)sizeof(input));
        
        int is_correct = checkAnswer(system, input);
        
        if (is_correct) {
            printf("\n正确！\n");
            printf("当前复习次数: %d\n", current_word->card.repetition);
            printf("当前间隔次数: %d\n", current_word->card.interval);
            printf("难度因子: %.2f\n", current_word->card.easiness_factor);
            updateWordStatus(system, 5);  // 完全正确给5分
            printf("下次复习间隔: %d次后\n", current_word->card.interval);
            printf("下次复习session: %d\n", current_word->card.next_session);
        } else {
            printf("\n错误！正确答案是: %s\n", current_word->word);
            printf("当前复习次数: %d\n", current_word->card.repetition);
            updateWordStatus(system, 0);  // 错误给0分
            printf("需要立即复习此单词\n");
        }
        
        words_studied++;

        printf("\n按Enter继续，输入'q'退出...");
        getchar(); // 消耗之前的换行符
        char ch = getchar();
        if (ch == 'q' || ch == 'Q') {
            break;
        }
    }

    if (words_studied > 0) {
        // 只有在实际学习了单词后才保存session和进度
        saveLastSession(system->current_session);
        saveWordsToFile(system, "progress.txt");
        printf("\n本次学习已完成，进度已保存。\n");
        Sleep(2000);
    } else {
        printf("\n当前没有需要学习的单词。\n");
        Sleep(2000);
    }
} 