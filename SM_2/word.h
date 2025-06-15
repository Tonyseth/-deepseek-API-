#ifndef WORD_H
#define WORD_H

#include "flashcard.h"
#include <stdio.h>
#include <string.h>

#define MAX_WORDS 1000
#define MAX_WORD_LENGTH 50
#define MAX_MEANING_LENGTH 100

typedef struct {
    char word[MAX_WORD_LENGTH];
    char meaning[MAX_MEANING_LENGTH];
    Flashcard card;          // SM2算法相关数据
    int level;              // 掌握程度
    int total_reviews;      // 总复习次数
    int error_count;        // 错误次数
} Word;

typedef struct {
    Word words[MAX_WORDS];
    int word_count;
    int current_index;
    unsigned int current_session;  // 当前学习会话数
    unsigned int last_saved_session;  // 上次保存的会话数
} WordSystem;

// 函数声明
void initWordSystem(WordSystem* system);
int addWord(WordSystem* system, const char* word, const char* meaning);
Word* getNextWord(WordSystem* system);
int checkAnswer(WordSystem* system, const char* answer);
void updateWordStatus(WordSystem* system, int grade);

// 添加文件操作相关函数声明
int loadWordsFromFile(WordSystem* system, const char* filename);
int saveWordsToFile(WordSystem* system, const char* filename);

// 新增：session相关函数声明
void saveLastSession(unsigned int session);
unsigned int loadLastSession(void);

#endif 