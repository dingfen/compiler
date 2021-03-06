#ifndef _DF_SYNTAX_H
#define _DF_SYNTAX_H

#include "lex.h"
#include "token.h"

#define SC_GLOBAL 1
#define SC_LOCAL 0
#define SC_MEMBER 0

/* 语法状态枚举 */
enum SynTaxState {
    SNTX_NUL,   // 空状态
    SNTX_SP,    // 空格
    SNTX_LF_HT, // 换行并缩进  声明 定义 语句结束都置为该状态
    SNTX_DELAY  // 延迟到取出下一个单词后确定输出格式
};

class Syntax {
public:
    Syntax(string filename);
    ~Syntax();

    /**
     * 功能：翻译单元，语法分析顶层
     * <translation_unit> ::= {<external_declaration>}<TK_EOF>
    */
    void translation_unit();

private:
    Lex lex;            // 内含有的词法分析器
    Token token;        // 当前分析到的词
    int syntax_state;   // 语法状态
    int syntax_level;   // 缩进级别
    bool is_read;

    /**
     * 功能： 解析外部声明
     * l： 存储类型 指明局部还是全局
     * ExternDeclaration 
     *  --> <type_specifier>(<TK_SEMICOLON>
     *      | <declarator><funcbody>
     *      | <declarator> [<TK_ASSIGN><initializer>]
     *       {<TK_COMMA><declarator>[<TK_ASSIGN><initializer>]}<TK_SEMICOLON>)
     */
    void external_declaration(int l);

    /**
     * 功能：解析类型区分符
     * 返回值：是否发现合法的类型区分符
     * <type_specifier>
     *  --> <KW_INT>|<KW_CHAR>|<KW_SHORT>|<KW_VOID>|<struct_specifier>
     */
    int type_specifier();

    /**
     * 功能：结构体类型区分符
     * <struct_specifier>
     *   --> <KW_STRUCT><IDENTIFIER><TK_BEGIN><struct_declaration_list><TK_END>
     *      | <KW_STRUCT><IDENTIFIER>
     */
    void struct_specifier();

    /**
     * 功能：结构体中声明的变量的全体
     * <struct_declaration_list> 
     *  --> <struct_declaration>{<struct_declaration>}
     */
    void struct_declaration_list();

    /**
     * 功能：结构体中声明的变量
     * <strcut_declaration>
     *  --> <type_specifier><struct_declarator_list><TK_SEMICOLON>
     * <struct_declarator_list>
     *  --> <declarator>{<TK_COMMA><declarator>}
     */
    void struct_declaration();

    /**
     * 功能：声明符的解析
     * <declarator>
     *  --> {<pointer>}<direct_declarator>
     */
    void declarator();

    /**
     * 功能：直接声明符解析
     * <direct_declarator>
     *  --> <IDENTIFIER><direct_declarator_postfix>
     */
    void direct_declarator();

    /**
     * 功能：直接声明符后缀
     * <direct_declarator_postfix>
     *  --> {<TK_OPENBR><TK_CINT><TK_CLOSBR>}
     *    | <TK_OPENBR><TK_CLOSBR>
     *    | <TK_OPENPA><parameter_type_list><TK_CLOSPA>
     *    | <TK_OPENPA><TK_CLOSPA>
     */
    void direct_declarator_postfix();

    /**
     * 功能：解析形参类型表
     * <parameter_type_list>
     *  --> <parameter_declaration>{<TK_COMMA><parameter_declaration>}
     * <parameter_declaration> --> <type_specifier><declarator>
     */
    void parameter_type_list();

    /**
     * 功能：函数体解析
     * <funcbody> --> <compound_statement>
     */
    void funcbody();

    /**
     * 功能：解析初值符
     * <initializer> --> <assignment_expression>
     */
    void initializer();

    /**
     * 功能：语句解析
     * <statement> --> <compound_statement>
     *  | <if_statement> | <return_statement> | <break_statement>
     *  | <continue_statement> | <for_statement> | <expression_statement>
     */
    void statement();

    /**
     * 功能：解析复合语句
     * <compound_statement> --> 
     *  <TK_BEGIN>{<declaration> | <statement>}<TK_END>
     */
    void compound_statement();

    /**
     * 功能：判断是否为类型区分符
     * v：单词编号
     */
    int is_type_specifier(TokenType v);

    /**
     * 功能：表达式语句解析
     * <expression_statement> --> <TK_SEMICOLON>|<expression><TK_SEMICOLON>
     */
    void expression_statement();

    /**
     * 功能：选择语句解析
     * <if_statement> --> <KW_IF><TK_OPENPA><expression><TK_CLOSPA><statement>
     *      [<KW_ELSE><statement>]
     */
    void if_statement();

    /**
     * 功能：循环语句解析
     * <for_statement> --> <KW_FOR><TK_OPENPA>
     *   <expression_statement><expression_statement><expression>
     *   <TK_CLOSPA><statement>
     */
    void for_statement();

    /**
     * 功能：continue语句解析
     */
    void continue_statement();

    /**
     * 功能：break语句解析
     */
    void break_statement();

    /**
     * 功能：return语句解析
     * <return_statement> --> <KW_RETURN><TK_SEMICOLON>
     *  | <KW_RETURN><expression><TK_SEMICOLON>
     */
    void return_statement();

    /**
     * 功能：解析表达式
     * <expression>--><assignment_expression>{<TK_COMMA><assignment_expression>}
     */
    void expression();

    /**
     * 功能：解析赋值表达式
     * <assignment_expression>
     *  --> <equality_expression>|<unary_expression><TK_ASSIGN><assignment_expression>
     */
    void assignment_expression();

    /**
     * 功能：解析相等类表达式
     * <equality_expression> --> <relational_expression>
     *  {<TK_EQ><relational_expression>}|<TK_NEQ><relational_expression>
     */
    void equality_expression();

    /**
     * 功能：关系表达式
     * <relational_expression> --> <additive_expression>{
     *  <TK_LT><additive_expression>
     *  | <TK_GT><additive_expression>
     *  | <TK_LEQ><additive_expression>
     *  | <TK_GEQ><additive_expression>}
     */
    void relational_expression();

    /**
     * 功能：加减类表达式
     * <additive_expression> --> <multiplicative_expression>
     *  {<TK_PLUS>|<TK_MINUS><multiplicative_expression>}
     */
    void additive_expression();

    /**
     * 功能：乘法除法表达式
     * <multiplicative_expression> --> <unary_expression>
     *  {<TK_STAR>|<TK_DIVIDE>|<TK_MOD><unary_expression>}
     */
    void multiplicative_expression();

    /**
     * 功能：一元表达式解析
     * <unary_expression> --> <postfix_expression>
     *  | <TK_AND>|<TK_STAR> <unary_expression>
     *  |<sizeof_expression>
     */
    void unary_expression();

    /**
     * 功能：解析sizeof表达式
     * <sizeof_expression> --> <KW_SIZEOF><TK_OPENPA><type_specifier><TK_CLOSEPA>
     */
    void sizeof_expression();

    /**
     * 功能：后缀表达式
     * <postfix_expression> --> <primary_expression>
     *  {<TK_OPENBR><expression><TK_CLOSEBR>|<TK_OPENPA><TK_CLOSEPA>
     *  |<TK_DOT><IDENTIFIER>|<TK_POINTSTO><IDENTIFIER>}
     */
    void postfix_expression();

    /**
     * 功能：解析初值表达式
     * <primary_expression> --> <IDENTIFIER>|<TK_CINT>|<TK_CSTR>|<TK_CCHAR>|
     *  <TK_OPENPA><expression><TK_CLOSEPA>
     */
    void primary_expression();

    /**
     * 功能：解析实参表达式
     * <argument_expression_list> --> <assignment_expression>
     *  { <TK_COMMA><assignment_expression> }
     */
    void argument_expression_list();

    /**
     * 功能：语法缩进
    */
    void syntax_indent();

    /**
     * 取下一个单词
     */
    Token next_token();

    /**
     * 跳过单词c，取下一个单词
     * 如果不是单词c，提示错误
     * c 要跳过的单词
    */
    void skip(TokenType c); 
};

#endif // _DF_SYNTAX_H