#include "word.h"
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

// 从文件加载单词
int loadWordsFromFile(WordSystem* system, const char* filename) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "r");
    if (err != 0) {
        printf("无法打开文件 %s\n", filename);
        return 0;
    }

    char line[MAX_WORD_LENGTH + MAX_MEANING_LENGTH + 2];
    char word[MAX_WORD_LENGTH];
    char meaning[MAX_MEANING_LENGTH];
    int loaded_count = 0;

    // 先清空缓冲区
    memset(line, 0, sizeof(line));
    memset(word, 0, sizeof(word));
    memset(meaning, 0, sizeof(meaning));

    while (fgets(line, sizeof(line), file) && loaded_count < MAX_WORDS) {
        // 跳过空行
        if (strlen(line) <= 1) continue;
        
        // 移除换行符和回车符
        line[strcspn(line, "\r\n")] = 0;
        
        // 跳过空白行
        if (strlen(line) == 0) continue;
        
        // 分割单词和释义（使用制表符分隔）
        char* tab = strchr(line, '\t');
        if (tab) {
            size_t word_len = tab - line;
            if (word_len >= MAX_WORD_LENGTH) {
                word_len = MAX_WORD_LENGTH - 1;
            }
            
            // 清理前后的空白字符
            while (word_len > 0 && isspace((unsigned char)line[word_len - 1])) {
                word_len--;
            }
            
            // 复制单词
            strncpy_s(word, MAX_WORD_LENGTH, line, word_len);
            word[word_len] = '\0';
            
            // 复制释义，跳过制表符后的空白字符
            char* meaning_start = tab + 1;
            while (*meaning_start && isspace((unsigned char)*meaning_start)) {
                meaning_start++;
            }
            
            strcpy_s(meaning, MAX_MEANING_LENGTH, meaning_start);
            
            // 清理释义末尾的空白字符
            char* end = meaning + strlen(meaning) - 1;
            while (end > meaning && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }
            
            // 确保单词和释义都不为空
            if (strlen(word) > 0 && strlen(meaning) > 0) {
                if (addWord(system, word, meaning)) {
                    loaded_count++;
                    printf("已加载: %s - %s\n", word, meaning); // 调试输出
                }
            }
        }
        
        // 清空缓冲区，准备下一行
        memset(line, 0, sizeof(line));
        memset(word, 0, sizeof(word));
        memset(meaning, 0, sizeof(meaning));
    }

    // 尝试加载进度文件
    FILE* progress_file;
    char progress_filename[] = "progress.txt";
    if (fopen_s(&progress_file, progress_filename, "r") == 0) {
        // 如果存在进度文件，读取进度
        char prog_line[256];
        while (fgets(prog_line, sizeof(prog_line), progress_file)) {
            char word[MAX_WORD_LENGTH];
            char meaning[MAX_MEANING_LENGTH];
            unsigned int repetition, interval;
            float easiness_factor;
            unsigned int session_count, next_session;

            if (sscanf_s(prog_line, "%s\t%s\t%u\t%u\t%f\t%u\t%u",
                word, (unsigned)sizeof(word),
                meaning, (unsigned)sizeof(meaning),
                &repetition,
                &interval,
                &easiness_factor,
                &session_count,
                &next_session) == 7) {
                
                // 查找对应的单词并更新其进度
                for (int i = 0; i < system->word_count; i++) {
                    if (strcmp(system->words[i].word, word) == 0) {
                        system->words[i].card.repetition = repetition;
                        system->words[i].card.interval = interval;
                        system->words[i].card.easiness_factor = easiness_factor;
                        system->words[i].card.session_count = session_count;
                        system->words[i].card.next_session = next_session;
                        break;
                    }
                }
            }
        }
        fclose(progress_file);
    }

    fclose(file);
    printf("\n成功从文件加载 %d 个单词\n", loaded_count);
    return loaded_count;
}

// 保存单词到文件
int saveWordsToFile(WordSystem* system, const char* filename) {
    if (system == NULL) return 0;

    FILE* file;
    errno_t err = fopen_s(&file, filename, "w");
    if (err != 0) {
        printf("无法创建文件 %s\n", filename);
        return 0;
    }

    int saved_count = 0;
    for (int i = 0; i < system->word_count; i++) {
        Word* word = &system->words[i];
        // 使用制表符分隔，确保所有数据都被正确保存
        if (fprintf(file, "%s\t%s\t%u\t%u\t%.2f\t%u\t%u\n", 
            word->word, 
            word->meaning, 
            word->card.repetition,
            word->card.interval,
            word->card.easiness_factor,
            word->card.session_count,
            word->card.next_session) > 0) {
            saved_count++;
        }
    }

    fclose(file);
    printf("成功保存 %d 个单词到文件\n", saved_count);
    return saved_count;
}