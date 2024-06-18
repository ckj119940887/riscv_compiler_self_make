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

    for(Node* N = Nd; N; N = N->Next) {
        genStmt(N);
        assert(Depth == 0);
    }

    printf("  ret\n");
}