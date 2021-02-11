#ifndef _DF_LEX_H
#define _DF_LEX_H

#include "token.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>


using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;

void color_print(char *fmt, ...);
void _color_token(Token token);
const std::unordered_map<string, TokenType> keyword2types{
    {"char",   TokenType::KW_CHAR},
    {"short",  TokenType::KW_SHORT},
    {"int",    TokenType::KW_INT},
    {"void",   TokenType::KW_VOID},
    {"struct", TokenType::KW_STRUCT},
    {"if",     TokenType::KW_IF},
    {"else",   TokenType::KW_ELSE},
    {"for",    TokenType::KW_FOR},
    {"continue",   TokenType::KW_CONTINUE},
    {"break",  TokenType::KW_BREAK},
    {"return", TokenType::KW_RETURN},
    {"sizeof", TokenType::KW_SIZEOF}};


class Lex {
public:
    Lex(string filename);
    ~Lex() {};

    /**
     * 词法涂色函数
    */ 
    void color_token();

    /**
     * 取单词 给到语法分析
     */
    Token get_token();

    /**
     * 获得取到的源码字符
     */
    void getch();

private:
    /**
     * 初始化
     */ 
    void init();
    
    /**
     * 清理工作
     */
    void cleanup();

    /**
     * 注释处理
     */
    void parse_comment();

    /**
     * 空白字符处理
     */
    void skip_white_space();

    /**
     * 预处理，忽略空白字符及注释
     */
    void preprocess();

    /**
     * 解析标识符
     */
    string parse_identifier();

    /**
     * 解析整形常量
     */
    string parse_num();

    /**
     * 解析字符常量和字符串常量
     * sep 单引号为 字符常量  双引号为字符串常量
     */
    string parse_string(char sep);

    /* private var */
    ifstream fin;
    int line_num;       // 行数
    int column_num;     // 列数
    char ch;            // 当前取得的字符

    bool is_read;
};

#endif // _DF_LEX_H