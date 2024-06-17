#include "rvcc.h"

// 语义分析与代码生成

int main(int Argc, char** Argv) {

    if(Argc != 2) {
        error("%s: invalid number of arguments %s\n", Argv[0], Argv[1]);
    }

    Token* Tok = tokenize(Argv[1]);

    Node* Nd = parse(Tok);

    codegen(Nd);

    return 0;
}