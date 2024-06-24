#include "rvcc.h"

static char* CurrentInput;

// 错误处理函数
void error(char* Fmt, ...)
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

// 错误出现的位置
void verrorAt(char* Loc, char* Fmt, va_list VA) {
    // 先输出源字符串
    fprintf(stderr, "%s\n", CurrentInput);

    // 计算出错位置, Loc是出错位置的指针，CurrentInput是当前输入的首地址
    int Pos = Loc - CurrentInput;
    // 将字符串补齐Pos位，补齐字符为空格
    fprintf(stderr, "%*s", Pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, Fmt, VA);
    fprintf(stderr, "\n");
    va_end(VA);
}

// 字符解析出错，退出程序
void errorAt(char* Loc, char* Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
    exit(1);
}

// Tok解析出错，并退出程序
void errorTok(Token* Tok, char* Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
    exit(1);
}

// 判断Tok是否等于指定值
bool equal(Token* Tok, char* Str) {
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
Token* skip(Token* Tok, char* Str) {
    if(!equal(Tok, Str))
        errorTok(Tok, "expect '%s'", Str);
    return Tok->Next;
}

// 返回TK_NUM的值
static int getNumber(Token* Tok)
{
    if(Tok->Kind != TK_NUM)
        errorTok(Tok, "expect a number");
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

// 判断Str是否以SubStr开头
bool startWith(char* Str, char* SubStr)
{
    // 比较LHS和RHS的N个字符是否相等
    return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

// 判断标记符的首字母规则
// [a-zA-Z_]
static bool isIdent1(char C) {
    return ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z') || (C == '_');
}

// 判断标记符的非首字母规则
// [a-zA-Z0-9_]
static bool isIdent2(char C) {
    return isIdent1(C) || ('0' <= C && C <= '9');
}

// 读取操作符
static int readPunct(char* Ptr)
{
    // 判断2字节操作符
    if(startWith(Ptr, "==") || startWith(Ptr, "!=") || 
       startWith(Ptr, "<=") || startWith(Ptr, ">="))
       return 2;

    // 判断1字节操作符
    return ispunct(*Ptr) ? 1 : 0;
}

// 判断是否为关键字
static bool isKeyword(Token* Tok) {
    //关键字列表
    static char* Kw[] = {"return", "if", "else", "for", "while"};

    //遍历关键字列表进行匹配
    //每个数组的元素是一样的，所以先算出总的长素，然后处以单独元素的长度
    for(int I = 0; I < sizeof(Kw) / sizeof(*Kw); I++) {
        if(equal(Tok, Kw[I]))
            return true;
    }

    return false;
}

// 遍历标识符链表，将所有关键字进行标识
static void convertKeywords(Token* Tok) {
    for(Token* T = Tok; T->Kind != TK_EOF; T = T->Next) {
        if(isKeyword(T)) {
            T->Kind = TK_KEYWORD;
        }
    }
}

// 终结符解析
Token* tokenize(char* P) {
    CurrentInput = P;
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
            // strtoul第二个参数会被设置为numerical value的下一个character(相当于P++)
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        //解析标识符或关键字
        //[a-zA-Z_][a-zA-Z0-9_]*
        if(isIdent1(*P)) {
            char* Start = P;
            do {
                ++P;
            } while(isIdent2(*P));

            // do-while循环里面P多加了一次
            Cur->Next = newToken(TK_IDENT, Start, P);
            Cur = Cur->Next;

            continue;
        }

        //解析操作符
        int PunctLen = readPunct(P);
        if(PunctLen) {
            Cur->Next = newToken(TK_PUNCT, P, P + PunctLen);
            Cur = Cur->Next;
            //指针前进PunctLen的长度位
            P = P + PunctLen;
            continue;
        }

        errorAt(P, "invalid token");
    }

    Cur->Next = newToken(TK_EOF, P, P);

    // 将所有关键字的终结符都标识为KEYWORD
    convertKeywords(Head.Next);

    return Head.Next;
}