#include "rvcc.h"

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

// 计算给定节点的绝对地址
// 报错，说明节点不在内存中
static void genAddr(Node *Nd) {
    if(Nd->Kind == ND_VAR) {
        // 偏移量=是两个字母在ASCII码表中的距离加1后乘以8，*8表示每个变量需要八个字节单位的内存
        int Offset = (Nd->Name - 'a' + 1) * 8;
        // 栈的增加是向下的
        printf("  addi a0, fp, %d\n", -Offset);
        return;
    } 

    error("not a lvalue");
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
    case ND_VAR:
        // 计算出变量的地址，然后存入a0
        genAddr(Nd);
        // 访问a0地址中存储的数据，存入到a0当中
        printf("  ld a0, 0(a0)\n");
        return;
    case ND_ASSIGN:
        // 左部是左值，保存值到地址
        genAddr(Nd->LHS);
        push();
        // 右部是右值，为表达式的值
        genExpr(Nd->RHS);
        pop("a1");
        printf("  sd a0, 0(a1)\n");
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
    case ND_EQ:
    case ND_NE:
        // a0 = a0 ^ a1
        printf("  xor a0, a0, a1\n");
        // a0 == a1
        // a0 = a0 ^ a1, sltiu a0, a0, 1
        // 等于0则置1
        if(Nd->Kind == ND_EQ)
            printf("  seqz a0, a0\n");
        // a0 != a1
        // a0 = a0 ^ a1, sltu a0, a0, 1
        // 不等于0则置1
        else
            printf("  snez a0, a0\n");
        return;
    case ND_LT:
        printf("  slt a0, a0, a1\n");
        return;
    case ND_LE:
        //a0<=a1等价于
        //a0=a1<a0,a0=a0^1
        printf("  slt a0, a1, a0\n");
        printf("  xori a0, a0, 1\n");
        return;
    default:
        break;
    }

    error("invalid expression");
}

// 生成语句
static void genStmt(Node* Nd) {
    if(Nd->Kind == ND_EXPR_STMT) {
        genExpr(Nd->LHS);
        return;
    }

    error("invalid statement");
}

void codegen(Node* Nd) {
    printf("  .global main\n");
    printf("main:\n");

    // 栈布局
    //-------------------------------// sp
    //              fp                  fp = sp-8
    //-------------------------------// fp
    //              'a'                 fp-8
    //              'b'                 fp-16
    //              ...
    //              'z'                 fp-208
    //-------------------------------// sp=sp-8-208
    //           表达式计算
    //-------------------------------//

    // Prologue, 前言
    // 将fp压入栈中，保存fp的值
    printf("  addi sp, sp, -8\n");
    printf("  sd fp, 0(sp)\n");
    // 将sp写入fp
    printf("  mv fp, sp\n");

    // 26个字母*8字节=208字节，栈腾出208字节的空间
    printf("  addi sp, sp, -208\n");

    for(Node* N = Nd; N; N = N->Next) {
        genStmt(N);
        assert(Depth == 0);
    }

    // Epilogue，后语
    // 将fp的值改写回sp
    printf("  mv sp, fp\n");
    // 将最早fp保存的值弹栈，恢复fp。
    printf("  ld fp, 0(sp)\n");
    printf("  addi sp, sp, 8\n");

    // 返回
    printf("  ret\n");
}