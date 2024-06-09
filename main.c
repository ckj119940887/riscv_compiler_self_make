#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//终结符类型
typedef enum {
    TK_PUNCT, //操作符
    TK_NUM, //数字
    TK_EOF, //终止符
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind Kind; //终结符种类
    Token* Next; //指向下一终结符
    int Val; //值
    char* Loc; //字符串中的位置
    int Len; //长度
};

// 错误处理函数
static void error(char* Fmt, ...)
{
    va_list VA;

    //获取Fmt后所有的参数
    va_start(VA, Fmt);

    //vfprintf可以输出va_list类型的参数
    vfprintf(stderr, Fmt, VA);
    fprintf(stderr, "\n");

    //清除VA
    va_end(VA);

    exit(1);
}

// 判断Tok是否等于指定值
static bool equal(Token* Tok, char* Str) {
    //int memcmp(const void *ptr1, const void *ptr2, size_t num);
    //ptr1: 指向第一个内存块的指针。
    //ptr2: 指向第二个内存块的指针。
    //num: 要比较的字节数。
    /*
    返回一个整数，根据比较结果可以是负数、零或正数：
    负数：如果在比较的前 num 个字节中，ptr1 所指向的内存块小于 ptr2 所指向的内存块。
    零：如果在比较的前 num 个字节中，ptr1 所指向的内存块等于 ptr2 所指向的内存块。
    正数：如果在比较的前 num 个字节中，ptr1 所指向的内存块大于 ptr2 所指向的内存块。
    */
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

// 跳过指定的Str
static Token* skip(Token* Tok, char* Str) {
    if(!equal(Tok, Str))
        error("expect '%s'", Str);
    return Tok->Next;
}

// 返回TK_NUM的值
static int getNumber(Token* Tok)
{
    if(Tok->Kind != TK_NUM)
        error("expect a number");
    return Tok->Val;
}

// 生成新的Token
static Token* newToken(TokenKind Kind, char* Start, char* End) {
    // 分配一个Token的内存空间
    Token* Tok = calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
}

// 终结符解析
static Token* tokenize(char* P) {
    Token Head = {};
    Token* Cur = &Head;

    while(*P) {
        //跳过所有空白、回车、\tab
        if(isspace(*P)) {
            ++P;
            continue;
        }

        //数字
        if(isdigit(*P)) {
            // Head仅作为头指针，用来表示链表入口，不存储信息
            Cur->Next = newToken(TK_NUM, P, P);
            
            Cur = Cur->Next;
            const char* OldPtr = P;
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        if(*P == '+' || *P == '-') {
            //操作符长度为1
            Cur->Next = newToken(TK_PUNCT, P, P + 1);
            Cur = Cur->Next;
            ++P;
            continue;
        }

        error("invalid token: '%c'", *P);
    }

    Cur->Next = newToken(TK_EOF, P, P);
    return Head.Next;
}

int main(int Argc, char** Argv) {

    if(Argc != 2) {
        error("%s: invalid number of arguments %s\n", Argv[0], Argv[1]);
    }

    Token* Tok = tokenize(Argv[1]);

    printf("  .global main\n");
    printf("main:\n");

    //将第一个num传入a0
    printf("li a0, %d\n", getNumber(Tok));
    Tok = Tok->Next;

    while(Tok->Kind != TK_EOF)
    {
        if(equal(Tok, "+")) {
            Tok = Tok->Next;
            printf("addi a0, a0, %d\n", getNumber(Tok));
            Tok = Tok->Next;
            continue;
        }

        Tok = skip(Tok, "-");
        printf("addi a0, a0, -%d\n", getNumber(Tok));
        Tok = Tok->Next;
    }

    printf("  ret\n");

    return 0;
}