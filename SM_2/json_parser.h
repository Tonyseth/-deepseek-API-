#ifndef JSON_PARSER_H
#define JSON_PARSER_H

// 解析DeepSeek API的JSON响应
int parse_deepseek_response(const char* response, char* score, char* suggestions);

#endif // JSON_PARSER_H 