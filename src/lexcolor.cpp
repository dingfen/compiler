#include "lex.h"


/**
 * 词法分析主函数入口
 * argv 为传入的文件名
 */ 
int main(int argc, char const *argv[])
{
    Lex lex(argv[1]);
    lex.color_token();
    return 0;
}
