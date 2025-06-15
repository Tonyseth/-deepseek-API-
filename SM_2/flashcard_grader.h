#ifndef LIBSPACEY_FLASHCARD_GRADER_H
#define LIBSPACEY_FLASHCARD_GRADER_H

#include "flashcard.h"

// 修改函数声明，使用unsigned int作为时间参数
void grade_flashcard(Flashcard* card, unsigned int grade, unsigned int current_session);

#endif 