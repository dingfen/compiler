
#include "syntax.h"


/**
 * 功能：语法缩进主函数
 */ 
int main(int argc, char const *argv[])
{
    Syntax syn(argv[1]);
    syn.translation_unit();
    return 0;
}
