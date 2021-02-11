#ifndef _DF_TOKEN_H
#define _DF_TOKEN_H
#include <string>

using std::string;

enum class TokenType {
    TK_PLUS = 0,        // +
    TK_MINUS,       // -
    TK_STAR,        // *
    TK_DIVIDE,      // /
    TK_MOD,         // %
    TK_EQ,          // ==
    TK_NEQ,         // !=
    TK_NOT,         // !
    TK_LT,          // <
    TK_LEQ,         // <=
    TK_GT,          // >
    TK_GEQ,         // >=
    TK_ASSIGN,      // =
    TK_POINTO,      // ->
    TK_DOT,         // .
    TK_SHARP,       // #
    TK_AND,         // &
    TK_OR,          // |
    TK_OPENPA,      // (
    TK_CLOSPA,      // )
    TK_OPENBR,      // [
    TK_CLOSBR,      // ]
    TK_BEGIN,       // {
    TK_END,         // }
    TK_SEMICOLON,   // ;
    TK_COMMA,       // ,
    TK_EOF,         // EOF

    /* 常量 */
    TK_CINT,        // 整型常量
    TK_CCHAR,       // 字符常量
    TK_CSTR,        // 字符串常量

    /* 关键字 */
    KW_CHAR,        // char
    KW_SHORT,       // short
    KW_INT,         // int
    KW_VOID,        // void
    KW_STRUCT,      // struct
    KW_IF,          // if
    KW_ELSE,        // else
    KW_FOR,         // for
    KW_CONTINUE,    // continue
    KW_BREAK,       // break
    KW_RETURN,      // return
    KW_SIZEOF,      // sizeof
    
    /* 标识符 */
    TK_IDENT
};


class Token {
public:

    /**
     * 设置该词的符号编码
     */ 
    void settype(TokenType type) {
        tkcode = type;
    }

    /**
     * 获得词的符号编号
     */
    TokenType type() {
        return tkcode;
    }
    
    /**
     * 赋值词的字符串
     */
    void setstr(const string& s) {
        spelling = s;
    }

    /**
     * 取得词的字符串
     */
    const string& str() {
        return spelling;
    }

private:
    TokenType tkcode;       // 词法符号编码
    string    spelling;     // 词的字符串
};

#endif // _DF_TOKEN_H