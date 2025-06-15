#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ascii_printer.h"

#define MAX_LINE_LEN 4096

int print_ascii_art(const char* filename) {
    FILE* fp;
    char line[MAX_LINE_LEN];
    char output[MAX_LINE_LEN];

    // 打开文件
    errno_t err = fopen_s(&fp, filename, "rb");
    if (err != 0) {
        printf("无法打开文件 %s\n", filename);
        return 1;
    }

    printf("\n");  // 开始前打印空行

    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        int out_pos = 0;

        // 处理每个字符
        for (size_t i = 0; i < len; i++) {
            char c = line[i];
            // 跳过换行符和回车符
            if (c == '\n' || c == '\r') continue;
            // 保持空格
            if (c == ' ') {
                output[out_pos++] = ' ';
                continue;
            }
            // 允许更多的ASCII艺术字符
            if (strchr("-_|/\\[]{}()<>.,`'\";:!~+*^?=", c) ||
                (c >= '0' && c <= '9') ||  // 允许数字
                (c >= 'a' && c <= 'z') ||  // 允许小写字母
                (c >= 'A' && c <= 'Z')) {  // 允许大写字母
                output[out_pos++] = c;
            }
            else {
                output[out_pos++] = ' ';
            }
        }

        // 添加字符串结束符
        output[out_pos] = '\0';

        // 移除尾部空格
        while (out_pos > 0 && output[out_pos - 1] == ' ') {
            output[--out_pos] = '\0';
        }

        // 打印处理后的行
        printf("%s\n", output);
    }

    printf("\n");  // 结束后打印空行
    fclose(fp);

    return 0;
}