#include "sentence_practice.h"
#include "gpt_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SENTENCES 100
#define MAX_SENTENCE_LENGTH 1024

typedef struct {
    char chinese[MAX_SENTENCE_LENGTH];
    char english[MAX_SENTENCE_LENGTH];
} Sentence;

static Sentence sentences[MAX_SENTENCES];
static int sentence_count = 0;

static int loadSentences(void) {
    FILE* file;
    char line[MAX_SENTENCE_LENGTH * 2];
    
    if (fopen_s(&file, "sentences.txt", "r") != 0) {
        printf("无法打开句子文件！\n");
        return 0;
    }
    
    sentence_count = 0;
    while (fgets(line, sizeof(line), file) && sentence_count < MAX_SENTENCES) {
        line[strcspn(line, "\n")] = 0;
        
        char* separator = strchr(line, '|');
        if (!separator) continue;
        
        *separator = '\0';
        
        strncpy_s(sentences[sentence_count].chinese, 
                 sizeof(sentences[sentence_count].chinese), 
                 line, _TRUNCATE);
                 
        strncpy_s(sentences[sentence_count].english, 
                 sizeof(sentences[sentence_count].english), 
                 separator + 1, _TRUNCATE);
                 
        sentence_count++;
    }
    
    fclose(file);
    return sentence_count;
}

int initSentencePractice(void) {
    return loadSentences();
}

void startSentencePractice(void) {
    system("cls");
    printf("\n=== 英语句子练习 ===\n");
    
    if (sentence_count == 0) {
        if (!loadSentences()) {
            printf("加载句子失败！\n按Enter继续...");
            while (getchar() != '\n');
            return;
        }
    }
    
    while (getchar() != '\n');
    
    int* used_sentences = (int*)calloc(sentence_count, sizeof(int));
    int remaining_sentences = sentence_count;
    
    while (1) {
        if (remaining_sentences == 0) {
            memset(used_sentences, 0, sentence_count * sizeof(int));
            remaining_sentences = sentence_count;
        }
        
        int index;
        do {
            index = rand() % sentence_count;
        } while (used_sentences[index]);
        
        used_sentences[index] = 1;
        remaining_sentences--;
        
        printf("\n中文句子：%s\n", sentences[index].chinese);
        printf("请输入英文翻译（输入'quit'退出）：\n> ");
        
        char user_input[1024];
        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            free(used_sentences);
            break;
        }
        user_input[strcspn(user_input, "\n")] = 0;
        
        if (strcmp(user_input, "quit") == 0) {
            free(used_sentences);
            break;
        }
        
        if (strlen(user_input) == 0) {
            continue;
        }
        
        printf("\n正在评估您的答案...\n");
        
        askGPTHelperJSON(user_input, sentences[index].english, sentences[index].chinese);
        
        printf("\n按Enter继续练习...");
        while (getchar() != '\n');
        
        system("cls");
        printf("\n=== 英语句子练习 ===\n");
    }
} 