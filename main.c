#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

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

static char* CurrentInput;

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

// 错误出现的位置
static void verrorAt(char* Loc, char* Fmt, va_list VA) {
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
static void errorAt(char* Loc, char* Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
    exit(1);
}

// Tok解析出错，并退出程序
static void errorTok(Token* Tok, char* Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
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

// 终结符解析
static Token* tokenize() {
    char *P = CurrentInput;
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

        //解析操作符
        if(ispunct(*P)) {
            //操作符长度为1
            Cur->Next = newToken(TK_PUNCT, P, P + 1);
            Cur = Cur->Next;
            ++P;
            continue;
        }

        errorAt(P, "invalid token");
    }

    Cur->Next = newToken(TK_EOF, P, P);
    return Head.Next;
}

// 生成AST
typedef enum {
    ND_ADD, //+
    ND_SUB, //-
    ND_MUL, //*
    ND_DIV, ///
    ND_NEG, //负号
    ND_NUM, //整形数字
} NodeKind;

// AST中二叉树节点
typedef struct Node Node;
struct Node {
    NodeKind Kind; //种类
    Node* LHS; //左部
    Node* RHS; //右部
    int Val; //ND_NUM种类的值
};

// 新建一个节点
static Node* newNode(NodeKind Kind) {
    Node* Nd = calloc(1, sizeof(Node));
    Nd->Kind = Kind;
    return Nd;
}

// 新建一个单叉树
static Node* newUnary(NodeKind Kind, Node* Expr) {
    Node* Nd = newNode(Kind);
    // 单叉树，直接关联到其左子树上，而不是右子树
    Nd->LHS = Expr;
    return Nd;
}

// 新建一个二叉树
static Node* newBinary(NodeKind Kind, Node* LHS, Node* RHS) {
    Node* Nd = newNode(Kind);
    Nd->LHS = LHS;
    Nd->RHS = RHS;
    return Nd;
}

// 新建一个数字节点
static Node* newNum(int val) {
    Node* Nd = newNode(ND_NUM);
    Nd->Val = val;
    return Nd;
}

// expr = mul("+" mul | "-" mul)*
// mul = unary("*" unary | "/" unary)*
// unary = ("+" | "-") unary | primary
// primary = "(" expr ")" | num
static Node* expr(Token **Rest, Token* Tok);
static Node* mul(Token **Rest, Token* Tok);
static Node* unary(Token **Rest, Token* Tok);
static Node* primary(Token **Rest, Token* Tok);

// 解析加减
// expr = mul("+" mul | "-" mul)*
static Node* expr(Token **Rest, Token* Tok) {
    // mul
    Node* Nd = mul(&Tok, Tok);

    //("+" mul | "-" mul)*
    while(true) {
        //"+" mul
        if(equal(Tok, "+")) {
            Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        //"-" mul
        if(equal(Tok, "-")) {
            Nd = newBinary(ND_SUB, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析乘除
// mul = unary("*" unary | "/" unary)*
static Node* mul(Token** Rest, Token* Tok) {
    // unary 
    Node* Nd = unary(&Tok, Tok);

    //("*" unary | "/" unary)*
    while(true) {
        // "*" unary
        if(equal(Tok, "*")) {
            Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        // "/" unary
        if(equal(Tok, "/")) {
            Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析一元运算
// unary = ("+" | "-") unary | primary
static Node* unary(Token** Rest, Token* Tok) {
    // "+" unary
    if(equal(Tok, "+"))
        return unary(Rest, Tok->Next);

    // "-" unary
    if(equal(Tok, "-"))
        return newUnary(ND_NEG, unary(Rest, Tok->Next));

    // primary
    return primary(Rest, Tok);
}

// 解析括号、数字
// primary = "(" expr ")" | num
static Node* primary(Token** Rest, Token* Tok) {
    // "(" expr ")"
    if(equal(Tok, "(")) {
        Node* Nd = expr(&Tok, Tok->Next);
        *Rest = skip(Tok, ")");
        return Nd;
    }

    if(Tok->Kind == TK_NUM) {
        Node* Nd = newNum(Tok->Val);
        *Rest = Tok->Next;
        return Nd;
    }

    errorTok(Tok, "expected an expression");
    return NULL;
}

// 语义分析与代码生成

// 记录栈的深度
static int Depth;

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push() {
    printf("  addi sp, sp, -8\n");
    printf("  sd a0, 0(sp)\n");
    Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char* Reg) {
    printf("  ld %s, 0(sp)\n", Reg);
    printf("  addi sp, sp, 8\n");
    Depth--;
}

// 表达式
static void genExpr(Node* Nd) {
    //加载数字到a0
    switch(Nd->Kind) {
    case ND_NUM:
        printf("  li a0, %d\n",Nd->Val);
        return;
    //对寄存器取反
    case ND_NEG:
        genExpr(Nd->LHS);
        printf("  neg a0, a0\n");
        return;
    default:
        break;
    }

    // 递归到最右节点
    genExpr(Nd->RHS);
    // 将结果压入栈
    push();
    // 递归到左节点
    genExpr(Nd->LHS);
    // 将结果弹栈到a1
    pop("a1");

    // 生成各个二叉树节点
    switch (Nd->Kind) {
    case ND_ADD: // + a0=a0+a1
        printf("  add a0, a0, a1\n");
        return;
    case ND_SUB: // - a0=a0-a1
        printf("  sub a0, a0, a1\n");
        return;
    case ND_MUL: // * a0=a0*a1
        printf("  mul a0, a0, a1\n");
        return;
    case ND_DIV: // / a0=a0/a1
        printf("  div a0, a0, a1\n");
        return;
    default:
        break;
    }

    error("invalid expression");
}

int main(int Argc, char** Argv) {

    if(Argc != 2) {
        error("%s: invalid number of arguments %s\n", Argv[0], Argv[1]);
    }

    CurrentInput = Argv[1];
    Token* Tok = tokenize();

    // 解析终结符流
    Node* Node = expr(&Tok, Tok);

    if(Tok->Kind != TK_EOF)
        errorTok(Tok, "extra token");
    
    printf("  .global main\n");
    printf("main:\n");

    genExpr(Node);

    printf("  ret\n");

    return 0;
}