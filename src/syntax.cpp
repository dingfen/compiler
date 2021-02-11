#include "syntax.h"


Syntax::Syntax(string filename) 
    : lex(filename), is_read(false), syntax_state(SNTX_NUL), syntax_level(0) {}

Syntax::~Syntax() {

}

/**
 * 取下一个单词
 */
Token Syntax::next_token() {
    if (!is_read) {
        lex.getch();
        is_read = true;
    }
    token = lex.get_token();
    syntax_indent();
    return token;
} 

/**
 * 跳过单词c，取下一个单词
 * 如果不是单词c，提示错误
 * c 要跳过的单词
*/
void Syntax::skip(TokenType c) {
    if (token.type() != c)
        cerr << "lack of "<< (int)c << endl;
    next_token();
}

/**
 * 功能：翻译单元，语法分析顶层
 * <translation_unit> ::= {<external_declaration>}<TK_EOF>
 */
void Syntax::translation_unit()  {
    next_token();
    while(token.type() != TokenType::TK_EOF) {
        external_declaration(SC_GLOBAL);
    }
}


/**
 * 功能： 解析外部声明
 * l： 存储类型 指明局部还是全局
 * ExternDeclaration 
 *  --> <type_specifier>(<TK_SEMICOLON>
 *      | <declarator><funcbody>
 *      | <declarator> [<TK_ASSIGN><initializer>]
 *       {<TK_COMMA><declarator>[<TK_ASSIGN><initializer>]}<TK_SEMICOLON>)
 */
void Syntax::external_declaration(int l) {
    if(!type_specifier()) {
        cerr << "<type id>" << endl;
    }
    if (token.type() == TokenType::TK_SEMICOLON) {
        next_token();
        return ;
    }
    while (1) {
        declarator();
        if (token.type() == TokenType::TK_BEGIN) {
            if (l == SC_LOCAL) {
                cerr << "nested func declarator unsupported." <<endl;
            }
            funcbody();
            break;
        }
        else {
            if (token.type() == TokenType::TK_ASSIGN) {
                next_token();
                initializer();
            }
            if (token.type() == TokenType::TK_COMMA) {
                next_token();
            }
            else {
                syntax_state = SNTX_LF_HT;
                skip(TokenType::TK_SEMICOLON);
                break;
            }
        }
    }
}

/**
 * 功能：解析类型区分符
 * 返回值：是否发现合法的类型区分符
 * <type_specifier>
 *  --> <KW_INT>|<KW_CHAR>|<KW_SHORT>|<KW_VOID>|<struct_specifier>
 */
int Syntax::type_specifier() {
    bool type_found = false;
    switch (token.type()) {
    case TokenType::KW_CHAR:
        type_found = true;
        syntax_state = SNTX_SP;
        next_token();
        break;
    case TokenType::KW_SHORT:
        type_found = true;
        syntax_state = SNTX_SP;
        next_token();
        break;
    case TokenType::KW_VOID:
        type_found = true;
        syntax_state = SNTX_SP;
        next_token();
        break;
    case TokenType::KW_INT:
        type_found = true;
        syntax_state = SNTX_SP;
        next_token();
        break;
    case TokenType::KW_STRUCT:
        syntax_state = SNTX_SP;
        type_found = true;
        struct_specifier();
        break;

    default:
        break;
    }
    return type_found;
}

/**
 * 功能：结构体类型区分符
 * <struct_specifier>
 *   --> <KW_STRUCT><IDENTIFIER><TK_BEGIN><struct_declaration_list><TK_END>;
 *      | <KW_STRUCT><IDENTIFIER>
 */
void Syntax::struct_specifier() {
    next_token();       
    auto type = token.type();       // should be identifier
    syntax_state = SNTX_DELAY;
    next_token();       

    // for define
    if (token.type() == TokenType::TK_BEGIN)  
        syntax_state = SNTX_LF_HT;
    // for sizeof(struct id)
    else if (token.type() == TokenType::TK_CLOSPA)
        syntax_state = SNTX_NUL;
    // for declarator
    else syntax_state = SNTX_SP;

    syntax_indent();

    if (type < TokenType::TK_IDENT)
        cerr << "struct identifier name" << endl;
    if (token.type() == TokenType::TK_BEGIN)
        struct_declaration_list();
    // skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：结构体中声明的变量的全体
 * <struct_declaration_list> 
 *  --> <struct_declaration>{<struct_declaration>}
 */
void Syntax::struct_declaration_list() {
    int maxalign, offset;

    syntax_state = SNTX_LF_HT;
    syntax_level++;

    next_token();
    while (token.type() != TokenType::TK_END) {
        struct_declaration();
    }
    skip(TokenType::TK_END);

    syntax_state = SNTX_LF_HT;
}

/**
 * 功能：结构体中声明的变量
 * <strcut_declaration>
 *  --> <type_specifier><struct_declarator_list><TK_SEMICOLON>
 * <struct_declarator_list>
 *  --> <declarator>{<TK_COMMA><declarator>}
 */
void Syntax::struct_declaration() {
    type_specifier();
    while (1) {
        declarator();

        if (token.type() == TokenType::TK_SEMICOLON)
            break;
        skip(TokenType::TK_COMMA);
    }
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：声明符的解析
 * <declarator>
 *  --> {<pointer>}<direct_declarator>
 */
void Syntax::declarator() {
    while(token.type() == TokenType::TK_STAR) {
        next_token();
    }
    direct_declarator();
}


/**
 * 功能：直接声明符解析
 * <direct_declarator>
 *  --> <IDENTIFIER><direct_declarator_postfix>
 */
void Syntax::direct_declarator()
{
    if (token.type() >= TokenType::TK_IDENT) {
        next_token();
    }
    else {
        cerr << "Identifier" << endl;
    }
    direct_declarator_postfix();
}


/**
 * 功能：直接声明符后缀
 * <direct_declarator_postfix>
 *  --> {<TK_OPENBR><TK_CINT><TK_CLOSBR>}
 *    | <TK_OPENBR><TK_CLOSBR>
 *    | <TK_OPENPA><parameter_type_list><TK_CLOSPA>
 *    | <TK_OPENPA><TK_CLOSPA>
 */
void Syntax::direct_declarator_postfix()
{
    int n;
    // for function
    if (token.type() == TokenType::TK_OPENPA) {
        parameter_type_list();
    }
    // for array
    else if (token.type() == TokenType::TK_OPENBR) {
        next_token();
        if (token.type() == TokenType::TK_CINT)
        {
            next_token();
            // n = tkvalue;
        }
        skip(TokenType::TK_CLOSBR);
        direct_declarator_postfix();
    }
}

/**
 * 功能：解析形参类型表
 * <parameter_type_list>
 *  --> <parameter_declaration>{<TK_COMMA><parameter_declaration>}
 * <parameter_declaration> --> <type_specifier><declarator>
 */
void Syntax::parameter_type_list()
{
    next_token();
    while(token.type() != TokenType::TK_CLOSPA) {
        if (!type_specifier()) {
            cerr << "invalid type specifier" << endl;
        }
        declarator();
        if (token.type() == TokenType::TK_CLOSPA)
            break;
        skip(TokenType::TK_COMMA);
    }
    syntax_state = SNTX_DELAY;
    skip(TokenType::TK_CLOSPA);
    if (token.type() == TokenType::TK_BEGIN)
        syntax_state = SNTX_LF_HT;  // function define
    else syntax_state = SNTX_NUL;   // function declaration

    syntax_indent();
}


/**
 * 功能：函数体解析
 * <funcbody> --> <compound_statement>
 */
void Syntax::funcbody()
{
    compound_statement();
}

/**
 * 功能：解析初值符
 * <initializer> --> <assignment_expression>
 */
void Syntax::initializer()
{
    assignment_expression();
}

/**
 * 功能：语句解析
 * <statement> --> <compound_statement>
 *  | <if_statement> | <return_statement> | <break_statement>
 *  | <continue_statement> | <for_statement> | <expression_statement>
 */
void Syntax::statement()
{
    switch (token.type()) {
    case TokenType::TK_BEGIN:
        compound_statement();
        break;
    case TokenType::KW_IF:
        if_statement();
        break;
    case TokenType::KW_RETURN:
        return_statement();
        break;
    case TokenType::KW_BREAK:
        break_statement();
        break;
    case TokenType::KW_CONTINUE:
        continue_statement();
        break;
    case TokenType::KW_FOR:
        for_statement();
        break;
    
    default:
        expression_statement();
        break;
    }
}

/**
 * 功能：解析复合语句
 * <compound_statement> --> 
 *  <TK_BEGIN>{<declaration> | <statement>}<TK_END>
 */
void Syntax::compound_statement()
{
    syntax_state = SNTX_LF_HT;
    syntax_level++;

    next_token();

    while (token.type() != TokenType::TK_END) {
        if (is_type_specifier(token.type()))
            external_declaration(SC_LOCAL);
        else 
            statement();
    }
    syntax_state = SNTX_LF_HT;
    next_token();
}


/**
 * 功能：判断是否为类型区分符
 * v：单词编号
 */
int Syntax::is_type_specifier(TokenType v) {
    switch (v) {
    case TokenType::KW_SHORT:
    case TokenType::KW_INT:
    case TokenType::KW_VOID:
    case TokenType::KW_STRUCT:
    case TokenType::KW_CHAR:
        return 1;
    
    default:
        break;
    }
    return 0;
}

/**
 * 功能：表达式语句解析
 * <expression_statement> --> <TK_SEMICOLON>|<expression><TK_SEMICOLON>
 */
void Syntax::expression_statement()
{
    if (token.type() != TokenType::TK_SEMICOLON)
        expression();
    
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：选择语句解析
 * <if_statement> --> <KW_IF><TK_OPENPA><expression><TK_CLOSPA><statement>
 *      [<KW_ELSE><statement>]
 */
void Syntax::if_statement()
{
    syntax_state = SNTX_SP;
    next_token();
    skip(TokenType::TK_OPENPA);
    expression();
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_CLOSPA);
    statement();
    if (token.type() == TokenType::KW_ELSE)
    {
        syntax_state = SNTX_LF_HT;
        next_token();
        statement();
    }
}


/**
 * 功能：循环语句解析
 * <for_statement> --> <KW_FOR><TK_OPENPA>
 *   <expression_statement><expression_statement><expression>
 *   <TK_CLOSPA><statement>
 */
void Syntax::for_statement()
{
    next_token();
    skip(TokenType::TK_OPENPA);
    if (token.type() != TokenType::TK_SEMICOLON)
        expression();
    skip(TokenType::TK_SEMICOLON);
    if (token.type() != TokenType::TK_SEMICOLON)
        expression();
    skip(TokenType::TK_SEMICOLON);
    if(token.type() != TokenType::TK_CLOSPA)
        expression();

    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_CLOSPA);
    statement();
}

/**
 * 功能：continue语句解析
 */
void Syntax::continue_statement()
{
    next_token();
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：break语句解析
 */
void Syntax::break_statement()
{
    next_token();
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：return语句解析
 * <return_statement> --> <KW_RETURN><TK_SEMICOLON>
 *  | <KW_RETURN><expression><TK_SEMICOLON>
 */
void Syntax::return_statement()
{
    syntax_state = SNTX_DELAY;
    next_token();
    if (token.type() == TokenType::TK_SEMICOLON)
        syntax_state = SNTX_NUL;
    else
        syntax_state = SNTX_SP;
    syntax_indent();

    if (token.type() != TokenType::TK_SEMICOLON)
        expression();
    syntax_state = SNTX_LF_HT;
    skip(TokenType::TK_SEMICOLON);
}

/**
 * 功能：解析表达式
 * <expression>--><assignment_expression>{<TK_COMMA><assignment_expression>}
 */
void Syntax::expression()
{
    while (1) {
        assignment_expression();
        if (token.type() != TokenType::TK_COMMA)
            break;
        next_token();
    }
}

/**
 * 功能：解析赋值表达式
 * <assignment_expression>
 *  --> <equality_expression>|<unary_expression><TK_ASSIGN><assignment_expression>
 */
void Syntax::assignment_expression()
{
    equality_expression();
    if(token.type() == TokenType::TK_ASSIGN) {
        next_token();
        assignment_expression();
    }
}

/**
 * 功能：解析相等类表达式
 * <equality_expression> --> <relational_expression>
 *  {<TK_EQ><relational_expression>}|<TK_NEQ><relational_expression>
 */
void Syntax::equality_expression()
{
    relational_expression();
    while (token.type() == TokenType::TK_EQ || 
        token.type() == TokenType::TK_NEQ) {
        next_token();
        relational_expression();
    }
}

/**
 * 功能：关系表达式
 * <relational_expression> --> <additive_expression>{
 *  <TK_LT><additive_expression>
 *  | <TK_GT><additive_expression>
 *  | <TK_LEQ><additive_expression>
 *  | <TK_GEQ><additive_expression>}
 */
void Syntax::relational_expression()
{
    additive_expression();
    while ((token.type() == TokenType::TK_LT ||
        token.type() == TokenType::TK_LEQ) ||
        (token.type() == TokenType::TK_GT || 
        token.type() == TokenType::TK_GEQ)) {
        next_token();
        additive_expression();
    }
}

/**
 * 功能：加减类表达式
 * <additive_expression> --> <multiplicative_expression>
 *  {<TK_PLUS>|<TK_MINUS><multiplicative_expression>}
 */
void Syntax::additive_expression()
{
    multiplicative_expression();
    while (token.type() ==TokenType::TK_PLUS || 
        token.type()==TokenType::TK_MINUS) {
        next_token();
        multiplicative_expression();
    }
}

/**
 * 功能：乘法除法表达式
 * <multiplicative_expression> --> <unary_expression>
 *  {<TK_STAR>|<TK_DIVIDE>|<TK_MOD><unary_expression>}
 */ 
void Syntax::multiplicative_expression()
{
    unary_expression();
    while (token.type() == TokenType::TK_STAR || 
        token.type() == TokenType::TK_DIVIDE || 
        token.type() == TokenType::TK_MOD) {
        next_token();
        unary_expression();
    }
}

/**
 * 功能：一元表达式解析
 * <unary_expression> --> <postfix_expression>
 *  | <TK_AND>|<TK_STAR> <unary_expression>
 *  |<sizeof_expression>
 */
void Syntax::unary_expression()
{
    switch (token.type())
    {
    case TokenType::TK_AND:
        next_token();
        unary_expression();
        break;
    case TokenType::TK_OR:
        next_token();
        unary_expression();
        break;
    case TokenType::TK_STAR:
        next_token();
        unary_expression();
        break;
    case TokenType::TK_PLUS:
        next_token();
        unary_expression();
        break;
    case TokenType::TK_MINUS:
        next_token();
        unary_expression();
        break;
    case TokenType::KW_SIZEOF:
        sizeof_expression();
        break;
    default:
        postfix_expression();
        break;
    }
}


/**
 * 功能：解析sizeof表达式
 * <sizeof_expression> --> <KW_SIZEOF><TK_OPENPA><type_specifier><TK_CLOSEPA>
 */
void Syntax::sizeof_expression()
{
    next_token();
    skip(TokenType::TK_OPENPA);
    type_specifier();
    skip(TokenType::TK_CLOSPA);
}

/**
 * 功能：后缀表达式
 * <postfix_expression> --> <primary_expression>
 *  {<TK_OPENBR><expression><TK_CLOSEBR>|<TK_OPENPA><TK_CLOSEPA>
 *  |<TK_DOT><IDENTIFIER>|<TK_POINTSTO><IDENTIFIER>}
 */
void Syntax::postfix_expression()
{
    primary_expression();
    while (1)
    {
        if (token.type() == TokenType::TK_DOT || 
            token.type() == TokenType::TK_POINTO) {
            next_token();
            // token |= SC_MEMBER;
            next_token();
        } 
        else if (token.type() == TokenType::TK_OPENBR) {
            next_token();
            expression();
            skip(TokenType::TK_CLOSBR);
        } else if (token.type() == TokenType::TK_OPENPA) {
            argument_expression_list();
        } else 
            break;
    }
} 

/**
 * 功能：解析初值表达式
 * <primary_expression> --> <IDENTIFIER>|<TK_CINT>|<TK_CSTR>|<TK_CCHAR>|
 *  <TK_OPENPA><expression><TK_CLOSEPA>
 */
void Syntax::primary_expression()
{
    TokenType t;
    switch (token.type())
    {
    case TokenType::TK_CINT:
    case TokenType::TK_CCHAR:
    case TokenType::TK_CSTR:
        next_token();
        break;
    case TokenType::TK_OPENPA:
        next_token();
        expression();
        skip(TokenType::TK_CLOSPA);
        break;
    default:
        t = token.type();
        next_token();
        if (t < TokenType::TK_CINT) {
            cerr << "Identifier or constant value." << endl;
        }
        break;
    }
}

/**
 * 功能：解析实参表达式
 * <argument_expression_list> --> <assignment_expression>
 *  { <TK_COMMA><assignment_expression> }
 */
void Syntax::argument_expression_list()
{
    next_token();
    if (token.type() != TokenType::TK_CLOSPA) {
        for(;;) {
            assignment_expression();
            if (token.type() == TokenType::TK_CLOSPA)
                break;
            skip(TokenType::TK_COMMA);
        }
    }
    skip(TokenType::TK_CLOSPA);
}


/**
 * 功能：缩进n个tab
 * n：缩进个数
 */
void print_tab(int n)
{
    for(int i = 0; i < n; i++)
        printf("\t");
}


/**
 * 功能：语法缩进
 */
void Syntax::syntax_indent()
{
    switch (syntax_state) {
    case SNTX_NUL:
        _color_token(token);
        break;
    case SNTX_SP:
        printf(" ");
        _color_token(token);
        break;
    case SNTX_LF_HT:{
        if (token.type() == TokenType::TK_END)
            syntax_level--;
        printf("\n");
        print_tab(syntax_level);
        _color_token(token);
        break;
    }
    case SNTX_DELAY:
        break;
    }
    syntax_state = SNTX_NUL;
}
