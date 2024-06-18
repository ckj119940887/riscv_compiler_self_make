#include "rvcc.h"

// 在解析时，全部的变量实例都被累加到这个列表里。
Obj* Locals;

// program = stmt*
// stmt = "return" expr ";" | exprStmt
// exprStmt = expr ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary("*" unary | "/" unary)*
// unary = ("+" | "-") unary | primary
// primary = "(" expr ")" | ident | num
static Node* stmt(Token** Rest, Token* Tok);
static Node* exprStmt(Token **Rest, Token* Tok);
static Node* expr(Token **Rest, Token* Tok);
static Node* assign(Token **Rest, Token* Tok);
static Node* equality(Token **Rest, Token* Tok);
static Node* relational(Token **Rest, Token* Tok);
static Node* add(Token **Rest, Token* Tok);
static Node* mul(Token **Rest, Token* Tok);
static Node* unary(Token **Rest, Token* Tok);
static Node* primary(Token **Rest, Token* Tok);

// 通过一个名称，查找本地变量
static Obj* findVar(Token* Tok) {
    // 查找Locals中是否存在同名变量
    for(Obj* Var = Locals; Var; Var = Var->Next) {
        if((strlen(Var->Name) == Tok->Len) && 
           !strncmp(Tok->Loc, Var->Name, Tok->Len)) {
                return Var;
           }
    }

    return NULL;
}

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

// 新建一个变量节点
static Node* newVarNode(Obj* Var) {
    Node* Nd = newNode(ND_VAR);
    Nd->Var = Var;
    return Nd;
}

// 链表中新增一个变量
static Obj* newLVar(char* Name) {
    Obj* Var = calloc(1, sizeof(Obj));
    Var->Name = Name;
    // 将变量插入头部
    Var->Next = Locals;
    Locals = Var;
    return Var;
}

// 解析语句
// stmt = "return" expr ";" | exprStmt
static Node* stmt(Token** Rest, Token* Tok) {
    //"return" expr ";"
    if(equal(Tok, "return")) {
        Node* Nd = newUnary(ND_RETURN, expr(&Tok, Tok->Next));
        *Rest = skip(Tok, ";");
        return Nd;
    }

    //exprStmt
    return exprStmt(Rest, Tok);
}

// 解析表达式语句
// exprStmt = expr ";"
static Node* exprStmt(Token** Rest, Token* Tok) {
    Node* Nd = newUnary(ND_EXPR_STMT, expr(&Tok, Tok));
    *Rest = skip(Tok, ";");
    return Nd;
}

// 解析表达式
// expr = assign
static Node* expr(Token **Rest, Token* Tok) {
    return assign(Rest, Tok);
}

// 解析赋值
// assign = equality ("=" assign)?
static Node* assign(Token **Rest, Token* Tok) {
    Node* Nd = equality(&Tok, Tok);

    // 可能存在递归赋值，如a=b=1
    // ("=" assign)?
    if(equal(Tok, "=")) {
        Nd = newBinary(ND_ASSIGN, Nd, assign(&Tok, Tok->Next));
    }

    *Rest = Tok;
    return Nd;
}

// 解析相等性
// equality = relational ("==" relational | "!=" relational)*
static Node* equality(Token **Rest, Token* Tok) {
    // relational
    Node* Nd = relational(&Tok, Tok);

    //("==" relational | "!=" relational)*
    while(true) {
        // "=="
        if(equal(Tok, "==")) {
            Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next));
            continue;
        }

        // "!="
        if(equal(Tok, "!=")) {
            Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析比较关系
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node* relational(Token** Rest, Token*Tok) {
    // add
    Node* Nd = add(&Tok, Tok);

    //("<" add | "<=" add | ">" add | ">=" add)*
    while(true) {
        // "<"
        if(equal(Tok, "<")) {
            Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next));
            continue;
        }

        // "<="
        if(equal(Tok, "<=")) {
            Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next));
            continue;
        }

        // ">"
        // X>Y等于Y<X
        if(equal(Tok, ">")) {
            Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd);
            continue;
        }

        // ">="
        if(equal(Tok, ">=")) {
            Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd);
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析加减
// add = mul ("+" mul | "-" mul)*
static Node* add(Token** Rest, Token* Tok) {
    // mul
    Node* Nd = mul(&Tok, Tok);

    //("+" mul | "-" mul)*
    while(true) {
        // "+" mul
        if(equal(Tok, "+")) {
            Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        // "-" mul
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

// 解析括号、数字、标识符
// primary = "(" expr ")" | ident | num
static Node* primary(Token** Rest, Token* Tok) {
    // "(" expr ")"
    if(equal(Tok, "(")) {
        Node* Nd = expr(&Tok, Tok->Next);
        // 这里实际上完成了Tok=Tok->Next->Next->*, 前面进行了多次递归调用，Rest
        // 的值都没有发生改变在最底层的rule中进行更新
        *Rest = skip(Tok, ")");
        return Nd;
    }

    // ident
    if(Tok->Kind == TK_IDENT) {
        // 查找变量
        Obj* Var = findVar(Tok);
        // strndup复制N个字符
        if(!Var)
            Var = newLVar(strndup(Tok->Loc, Tok->Len));

        *Rest = Tok->Next;
        return newVarNode(Var);
    }

    if(Tok->Kind == TK_NUM) {
        Node* Nd = newNum(Tok->Val);
        // 这里实际上完成了Tok=Tok->Next->Next->*, 前面进行了多次递归调用，Rest
        // 的值都没有发生改变在最底层的rule中进行更新
        *Rest = Tok->Next;
        return Nd;
    }

    errorTok(Tok, "expected an expression");
    return NULL;
}

// 语法解析入口函数
// program = stmt*
Function *parse(Token *Tok) {
    Node Head = {};
    Node* Cur = &Head;

    // stmt*
    while(Tok->Kind != TK_EOF) {
        Cur->Next = stmt(&Tok, Tok);
        Cur = Cur->Next;
    }

    // 函数体存储语句的AST，Locals存储变量
    Function* Prog = calloc(1, sizeof(Function));
    Prog->Body = Head.Next;
    Prog->Locals = Locals;

    return Prog;
}