#include "word.h"
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "flashcard_grader.h"

#define SESSION_FILE "session.txt"

unsigned int loadLastSession() {
    FILE* file;
    unsigned int session = 1;
    
    if (fopen_s(&file, SESSION_FILE, "r") == 0) {
        fscanf_s(file, "%u", &session);
        fclose(file);
    }
    return session;
}

void saveLastSession(unsigned int session) {
    FILE* file;
    if (fopen_s(&file, SESSION_FILE, "w") == 0) {
        fprintf(file, "%u", session);
        fclose(file);
    }
}

void initWordSystem(WordSystem* system) {
    system->word_count = 0;
    system->current_index = -1;
    system->current_session = loadLastSession();
    system->last_saved_session = system->current_session;
}

int addWord(WordSystem* system, const char* word, const char* meaning) {
    if (system->word_count >= MAX_WORDS) {
        return 0;
    }

    Word* new_word = &system->words[system->word_count];
    strcpy_s(new_word->word, MAX_WORD_LENGTH, word);
    strcpy_s(new_word->meaning, MAX_MEANING_LENGTH, meaning);
    
    // 初始化Flashcard数据
    flashcard_init(&new_word->card);
    
    new_word->level = 0;
    new_word->total_reviews = 0;
    new_word->error_count = 0;

    system->word_count++;
    return 1;
}

// 获取下一个需要复习的单词
Word* getNextWord(WordSystem* system) {
    if (system->word_count == 0) {
        return NULL;
    }

    // 首先检查是否有本session需要复习的单词
    int review_indices[MAX_WORDS] = {0};
    int review_count = 0;
    
    for (int i = 0; i < system->word_count; i++) {
        // 检查是否是本session需要复习的单词
        if (system->words[i].card.next_session <= system->current_session && 
            system->words[i].total_reviews > 0) {  // 确保是已学习过的单词
            review_indices[review_count++] = i;
        }
    }
    
    // 如果有需要复习的单词，随机选择一个
    if (review_count > 0) {
        int random_index = review_indices[rand() % review_count];
        system->current_index = random_index;
        return &system->words[random_index];
    }
    
    // 只有在没有需要复习的单词时，才学习新单词
    int unlearned_indices[MAX_WORDS] = {0};
    int unlearned_count = 0;
    
    for (int i = 0; i < system->word_count; i++) {
        if (system->words[i].total_reviews == 0) {
            unlearned_indices[unlearned_count++] = i;
        }
    }
    
    // 如果有未学习的单词，随机选择一个
    if (unlearned_count > 0) {
        int random_index = unlearned_indices[rand() % unlearned_count];
        system->current_index = random_index;
        return &system->words[random_index];
    }
    
    // 如果既没有需要复习的也没有未学习的单词，返回NULL
    return NULL;
}

// 检查答案是否正确（不区分大小写）
int checkAnswer(WordSystem* system, const char* answer) {
    if (system->current_index < 0) {
        return 0;
    }

    Word* current_word = &system->words[system->current_index];
    char correct[MAX_WORD_LENGTH];
    char user_answer[MAX_WORD_LENGTH];
    
    // 转换为小写进行比较
    strcpy_s(correct, MAX_WORD_LENGTH, current_word->word);
    strcpy_s(user_answer, MAX_WORD_LENGTH, answer);
    
    for (int i = 0; correct[i]; i++) {
        correct[i] = tolower(correct[i]);
    }
    for (int i = 0; user_answer[i]; i++) {
        user_answer[i] = tolower(user_answer[i]);
    }

    return strcmp(correct, user_answer) == 0;
}

// 更新单词状态
void updateWordStatus(WordSystem* system, int grade) {
    if (system->current_index < 0) {
        return;
    }

    Word* current_word = &system->words[system->current_index];
    current_word->total_reviews++;
    
    if (grade < 3) {
        current_word->error_count++;
    }

    // 使用SM2算法更新单词状态和计算下次复习时间
    grade_flashcard(&current_word->card, grade, system->current_session);
} 