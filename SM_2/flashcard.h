#ifndef LIBSPACEY_FLASHCARD_H
#define LIBSPACEY_FLASHCARD_H

// 默认值定义
#define DEFAULT_REPETITION 0
#define DEFAULT_INTERVAL 1
#define DEFAULT_EASINESS_FACTOR 2.5f

// Flashcard结构体
typedef struct {
    unsigned int repetition;
    unsigned int interval;      // 现在表示下次复习前需要经过的学习次数
    float easiness_factor;
    unsigned int session_count; // 新增：记录程序启动次数
    unsigned int next_session;  // 新增：在第几次启动时复习
} Flashcard;

// 函数声明
void flashcard_init(Flashcard* card);

#endif