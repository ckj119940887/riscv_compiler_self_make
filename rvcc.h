// 使用POSIX.1标准
// 使用了strndup函数
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

//
// 终结符分析，词法分析
//

//终结符类型
typedef enum {
    TK_IDENT, //标识符，可以为变量名和函数名等
    TK_PUNCT, //操作符
    TK_KEYWORD, //关键字
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

// 去除了static用以在多个文件间访问
// 报错函数
void error(char *Fmt, ...);
void errorAt(char *Loc, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);
// 判断Token与Str的关系
bool equal(Token *Tok, char *Str);
Token *skip(Token *Tok, char *Str);
// 词法分析
Token *tokenize(char *Input);

//
// 生成AST（抽象语法树），语法解析
//

// 生成AST(抽象语法树)
typedef enum {
    ND_ADD, //+
    ND_SUB, //-
    ND_MUL, //*
    ND_DIV, ///
    ND_NEG, //负号
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ASSIGN, // 赋值
    ND_RETURN, // 返回
    ND_EXPR_STMT, // 表达式语句
    ND_VAR, // 变量
    ND_NUM, // 整形数字
} NodeKind;

typedef struct Node Node;

// 本地变量
typedef struct Obj Obj;
struct Obj {
    Obj* Next; // 指向下一对象
    char* Name; // 变量名
    int Offset; // fp的偏移量
};

// AST中二叉树节点
struct Node {
    NodeKind Kind; //种类
    Node* Next; // 下一语句
    Node* LHS; //左部
    Node* RHS; //右部
    Obj* Var; //存储ND_VAR的种类
    int Val; //ND_NUM种类的值
};

//函数
typedef struct Function Function;
struct Function {
    Node* Body; //函数体
    Obj* Locals; //本地变量
    int StackSize; //栈大小
};

// 语法解析入口函数
Function* parse(Token *Tok);

//
// 语义分析与代码生成
//

// 代码生成入口函数
void codegen(Function *Prog);