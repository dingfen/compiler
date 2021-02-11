#include "lex.h"
#include <unistd.h>
#include <cctype>


#include <cstdarg>
#include <cstdlib>

#define BLUE   34
#define YELLOW 33
#define RED    31
#define GREEN  32

/**
 * 词法着色
 */
void color_print(char *fmt, ...)
{
    va_list ap;
    char buf[256];
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    printf("%s", buf);
    va_end(ap);
}


void _color_token(Token token)
{
    char fmt[256];
    if (token.type() >= TokenType::TK_IDENT)  {// 标识符 为白色
        sprintf(fmt, "%%s");
    }
    else if (token.type() >= TokenType::KW_CHAR)  {// 关键字 蓝色 34
        sprintf(fmt, "\033[%dm%%s\033[0m", BLUE);
    }
    else if (token.type() >= TokenType::TK_CINT) {// 常量 黄色 33
        sprintf(fmt, "\033[%dm%%s\033[0m", YELLOW);
    }
    else if (token.type() == TokenType::TK_POINTO) {
        sprintf(fmt, "\033[%dm%%s\033[0m", GREEN);
    }
    else {// 运算符等  红色 31
        sprintf(fmt, "\033[%dm%%s\033[0m", RED);
    }
    color_print(fmt, token.str().c_str());
}


void Lex::init() {
    this->line_num = 1;
    this->column_num = 0;
    this->is_read = false;
}

Lex::Lex(string filename) {
    fin.open(filename, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        cerr << "Can not open the SC file: " << filename << endl;
    }
    init();
}


void Lex::color_token() {
    #ifdef COLOR_TOKEN
    Token t;
    getch();
    do {
        t = get_token();
        _color_token(t);
    } while (!fin.eof());
    printf("\n 代码行数：%d行, 代码列数：%d列\n", line_num, column_num);

    cleanup();
    fin.close();
    #endif
}


void Lex::getch()
{
    fin.get(ch);
    column_num ++;
}

/**
 * 清理工作
 */
void Lex::cleanup()
{
    // int i;
    // printf("\ntktable.count=%d\n", tktable.count);
    // for(i = TK_IDENT; i < tktable.count; i++)
    // {
    //     free(tktable.data[i]);
    // }
    // free(tktable.data);
}


void Lex::parse_comment() {
    getch();
    do {
        do {
            if (ch == '\n' || ch == '*' || ch == EOF)
                break;
            else 
                getch();
        } while (1);
        if (ch == '\n') {
            line_num ++;
            column_num = 0;
            getch();
        }
        else if (ch == '*') {
            getch();
            if (ch == '/') {
                getch();
                return;
            }
        }
        else {
            cerr << "No End_Of_File found at the end of file." << endl;
            return;
        }
    }while(1);
}


void Lex::skip_white_space()
{
    while(ch == ' ' || ch == '\t' || ch == '\r' ||ch == '\n')
    {
        if (ch == '\n')
        {
            // getch();
            line_num++;
            column_num = 0;
        }
        // printf("%c", ch);
        getch();
    }
} 


void Lex::preprocess() {
    while (1) {
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            skip_white_space();
        else if (ch == '/') {
            // 看是否是注释 仅支持 /* */
            getch();
            if (ch == '*') {
                parse_comment();
            }
            else {
                fin.unget();
                column_num--;
                ch = '/';
                break;
            }
        } 
        else 
            break;
    }
}


/**
 * 解析标识符
 */
string Lex::parse_identifier()
{
    string tkstr;
    tkstr.push_back(ch);
    getch();

    while (isalnum(ch) || ch == '_')
    {
        tkstr.push_back(ch);
        getch();
    }
    
    return tkstr;
}

/**
 * 解析整形常量
 */
string Lex::parse_num()
{
    string tkstr;
    do
    {
        tkstr.push_back(ch);
        getch();
    } while (isdigit(ch));
    if (ch =='.')
    {
        do
        {
            tkstr.push_back(ch);
            getch();
        } while (isdigit(ch));   
    }
    return tkstr;
} 

/**
 * 解析字符常量和字符串常量
 * sep 单引号为 字符常量  双引号为字符串常量
 */
string Lex::parse_string(char sep)
{
    char c;
    string tkstr;
    tkstr.push_back(ch);
    getch();
    for(;;) {
        if (ch == sep) {
            tkstr.push_back(ch);
            break;
        }
        else if (ch == '\\') {
            tkstr.push_back(ch);
            // dynstring_append(&sourcestr, ch);
            getch();
            // 解析转义字符
            switch (ch) {
            case '0':
                c = '\0';
                break;
            case 'b':
                c = '\b';
                break;
            case 'n':
                c = '\n';
                break;
            case '\'':
                c = '\'';
                break;
            case '\"':
                c = '\"';
                break;
            default:
                c = ch;
                cerr << "illegal escape character: \'\\" << c << "\'" << endl;
                break;
            }
            // tkstr.push_back(c);
            tkstr.push_back(ch);
            getch();
        }
        else {
            tkstr.push_back(ch);
            getch();
        }
    }
    getch();
    return tkstr;
} 

/**
 * 取单词主程序
 */
Token Lex::get_token() {
    Token t;
    string s;
    if (fin.eof()) {
        t.settype(TokenType::TK_EOF);
        return t;
    }
    preprocess();
    if (isalpha(ch) || ch == '_') {
        // TKWord *tp;
        s = parse_identifier();
        if (keyword2types.find(s) != keyword2types.end())
            t.settype(keyword2types.at(s));
        else
            t.settype(TokenType::TK_IDENT);
        t.setstr(s);
    }
    else if (isdigit(ch)) {
        s = parse_num();
        t.settype(TokenType::TK_CINT);
        t.setstr(s);
    }
    else {
        switch (ch) {
        case '+':
            t.settype(TokenType::TK_PLUS);
            t.setstr("+");
            getch();
            break;
        case '-':
            getch();
            if (ch == '>') {
                t.settype(TokenType::TK_POINTO);
                t.setstr("->");
                getch();
            }
            else {
                t.settype(TokenType::TK_MINUS);
                t.setstr("-");
            }
            break;
        case '/':
            t.settype(TokenType::TK_DIVIDE);
            t.setstr("/");
            getch();
            break;
        case '%':
            t.settype(TokenType::TK_MOD);
            t.setstr("%");
            getch();
            break;
        case '=':
            getch();
            if (ch == '=') {
                t.settype(TokenType::TK_EQ);
                t.setstr("==");
                getch();
            }
            else {
                t.settype(TokenType::TK_ASSIGN);
                t.setstr("=");
            }
            break;
        case '!':
            getch();
            if (ch == '=') {
                t.settype(TokenType::TK_NEQ);
                t.setstr("!=");
                getch();
            }
            else {
                t.settype(TokenType::TK_NOT);
                t.setstr("!");
            }   
            break;
        case '<':
            getch();
            if (ch == '=') {
                t.settype(TokenType::TK_LEQ);
                t.setstr("<=");
                getch();
            }
            else {
                t.settype(TokenType::TK_LT);
                t.setstr("<");
            }
            break;
        case '>':
            getch();
            if (ch == '=') {
                t.settype(TokenType::TK_GEQ);
                t.setstr(">=");
                getch();
            }
            else {
                t.settype(TokenType::TK_GT);
                t.setstr(">");
            }
            break;
        case '.':
            getch();
            t.settype(TokenType::TK_DOT);
            t.setstr(".");
            break;
        case '#':
            getch();
            t.settype(TokenType::TK_SHARP);
            t.setstr("#");
            break;
        case '&':
            t.settype(TokenType::TK_AND);
            t.setstr("&");
            getch();
            break;
        case '|':
            t.settype(TokenType::TK_OR);
            t.setstr("|");
            getch();
            break;
        case ';':
            t.settype(TokenType::TK_SEMICOLON);
            t.setstr(";");
            getch();
            break;
        case '(':
            t.settype(TokenType::TK_OPENPA);
            t.setstr("(");
            getch();
            break;
        case '[':
            t.settype(TokenType::TK_OPENBR);
            t.setstr("[");
            getch();
            break;
        case '{':
            t.settype(TokenType::TK_BEGIN);
            t.setstr("{");
            getch();
            break;
        case ')':
            t.settype(TokenType::TK_CLOSPA);
            t.setstr(")");
            getch();
            break;
        case ']':
            t.settype(TokenType::TK_CLOSBR);
            t.setstr("]");
            getch();
            break;
        case '}':
            t.settype(TokenType::TK_END);
            t.setstr("}");
            getch();
            break;
        case ',':
            t.settype(TokenType::TK_COMMA);
            t.setstr(",");
            getch();
            break;
        case '*':
            t.settype(TokenType::TK_STAR);
            t.setstr("*");
            getch();
            break;
        case '\'':
            s = parse_string(ch);
            t.settype(TokenType::TK_CCHAR);
            t.setstr(s);
            // tkvalue = *(char *)tkstr.data;
            break;
        case '\"':
            s = parse_string(ch);
            t.settype(TokenType::TK_CSTR);
            t.setstr(s);
            break;
        default:
            cerr << "illegal word! Lexical cannot recognise " << ch << endl;
            getch();
            break;
        }
    }
    #ifdef __SYNTAX_INDENT
        syntax_indent();
    #endif
    return t;
}
