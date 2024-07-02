#include "rvcc.h"

// 记录栈的深度
static int Depth;

// 代码段标号计数
static int count(void) {
    static int I = 1;
    return I++;
}

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push() {
    printf("  # 压栈，将a0的值压入栈顶\n");
    printf("  addi sp, sp, -8\n");
    printf("  sd a0, 0(sp)\n");
    Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char* Reg) {
    printf("  # 弹栈，将栈顶的值存入%s\n", Reg);
    printf("  ld %s, 0(sp)\n", Reg);
    printf("  addi sp, sp, 8\n");
    Depth--;
}

// 对齐到Align的整数倍
static int alignTo(int N, int Align) {
    return (N + Align - 1) / Align * Align;
}

// 计算给定节点的绝对地址
// 报错，说明节点不在内存中
static void genAddr(Node *Nd) {
    if(Nd->Kind == ND_VAR) {
        //偏移量是相对于fp的
        printf("  # 获取变量%s的栈内地址为%d(fp)\n", Nd->Var->Name, Nd->Var->Offset);
        printf("  addi a0, fp, %d\n", Nd->Var->Offset);
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
        printf("  # 对a0值进行取反\n");
        printf("  neg a0, a0\n");
        return;
    case ND_VAR:
        // 计算出变量的地址，然后存入a0
        genAddr(Nd);
        // 访问a0地址中存储的数据，存入到a0当中
        printf("  # 读取a0中存放的地址，得到的值存入a0\n");
        printf("  ld a0, 0(a0)\n");
        return;
    case ND_ASSIGN:
        // 左部是左值，保存值到地址
        genAddr(Nd->LHS);
        push();
        // 右部是右值，为表达式的值
        genExpr(Nd->RHS);
        pop("a1");
        printf("  # 将a0的值，写入到a1中存放的地址\n");
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
        printf("  # a0+a1，结果写入a0\n");
        printf("  add a0, a0, a1\n");
        return;
    case ND_SUB: // - a0=a0-a1
        printf("  # a0-a1，结果写入a0\n");
        printf("  sub a0, a0, a1\n");
        return;
    case ND_MUL: // * a0=a0*a1
        printf("  # a0×a1，结果写入a0\n");
        printf("  mul a0, a0, a1\n");
        return;
    case ND_DIV: // / a0=a0/a1
        printf("  # a0÷a1，结果写入a0\n");
        printf("  div a0, a0, a1\n");
        return;
    case ND_EQ:
    case ND_NE:
        // a0 = a0 ^ a1
        printf("  # 判断是否a0%sa1\n", Nd->Kind == ND_EQ ? "=" : "≠");
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
        printf("  # 判断a0<a1\n");
        printf("  slt a0, a0, a1\n");
        return;
    case ND_LE:
        //a0<=a1等价于
        //a0=a1<a0,a0=a0^1
        printf("  # 判断是否a0≤a1\n");
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
    switch(Nd->Kind) {
    case ND_IF: {
        //代码段计数
        int C = count();
        printf("\n# =====分支语句%d==============\n", C);
        //生成条件内语句
        genExpr(Nd->Cond);
        // 判断结果是否为0，为0则跳转到else标签
        printf("  # 若a0为0，则跳转到分支%d的.L.else.%d段\n", C, C);
        printf("  beqz a0, .L.else.%d\n", C);
        // 生成符合条件后的语句
        printf("\n# Then语句%d\n", C);
        genStmt(Nd->Then);
        // 执行完后跳转到if语句后面的语句
        printf("  # 跳转到分支%d的.L.end.%d段\n", C, C);
        printf("  j .L.end.%d\n", C);
        // else代码块，else可能为空，故输出标签
        printf("\n# Else语句%d\n", C);
        printf("# 分支%d的.L.else.%d段标签\n", C, C);
        printf(".L.else.%d:\n", C);
        // 生成不符合条件后的语句
        if (Nd->Els)
            genStmt(Nd->Els);
        // 结束if语句，继续执行后面的语句
        printf("\n# 分支%d的.L.end.%d段标签\n", C, C);
        printf(".L.end.%d:\n", C);
        return;
    }
    // 生成for循环语句
    case ND_FOR: {
        //代码段计数
        int C = count();
        printf("\n# =====循环语句%d===============\n", C);
        //生成初始化语句
        if(Nd->Init) {
            printf("\n# Init语句%d\n", C);
            genStmt(Nd->Init);
        }
        //输出循环头部标签
        printf("\n# 循环%d的.L.begin.%d段标签\n", C, C);
        printf(".L.begin.%d:\n", C);
        //处理循环条件语句
        printf("# Cond表达式%d\n", C);
        if(Nd->Cond) {
            //生成条件循环语句
            genExpr(Nd->Cond);
            //判断结构是否为0，为0则跳转到结束部分
            printf("  # 若a0为0，则跳转到循环%d的.L.end.%d段\n", C, C);
            printf("  beqz a0, .L.end.%d\n", C);
        }
        //生成循环体语句
        printf("\n# Then语句%d\n", C);
        genStmt(Nd->Then);
        //处理循环递增语句
        if(Nd->Inc) {
            printf("\n# Inc语句%d\n", C);
            genExpr(Nd->Inc);
        }
        //跳转到循环头部
        printf("  # 跳转到循环%d的.L.begin.%d段\n", C, C);
        printf("  j .L.begin.%d\n", C);
        //输出循环尾部标签
        printf("\n# 循环%d的.L.end.%d段标签\n", C, C);
        printf(".L.end.%d:\n", C);
        return;
    }
    // 生成代码块
    case ND_BLOCK:
        for(Node* N = Nd->Body; N; N = N->Next)
            genStmt(N);
        return;
    // 生成return语句
    case ND_RETURN:
        printf("# 返回语句\n");
        genExpr(Nd->LHS);
        // 无条件跳转语句，跳转到.L.return段
        // j offset是 jal x0, offset的别名指令
        printf(" # 跳转到.L.return段\n");
        printf("  j .L.return\n");
        return;
    // 生成表达式语句
    case ND_EXPR_STMT:
        genExpr(Nd->LHS);
        return;
    default:
        break;
    }

    error("invalid statement");
}

// 根据变量的链表计算出偏移量
static void assignLVarOffsets(Function* Prog) {
    int Offset = 0;

    //读取所有变量
    for(Obj* Var = Prog->Locals; Var; Var = Var->Next) {
        // 每个变量分配8个字节
        Offset += 8;
        // 为每个变量赋一个偏移量，即栈中地址
        Var->Offset = -Offset;
    }

    // 将栈对齐到16字节
    Prog->StackSize = alignTo(Offset, 16);
}

void codegen(Function* Prog) {
    assignLVarOffsets(Prog);
    printf("  # 定义全局main段\n");
    printf("  .global main\n");
    printf("\n# =====程序开始===============\n");
    printf("# main段标签，也是程序入口段\n");
    printf("main:\n");

    // 栈布局
    //-------------------------------// sp
    //              fp                  
    //-------------------------------// fp = sp-8
    //              变量                 
    //-------------------------------// fp = sp-8-StackSize
    //           表达式计算
    //-------------------------------//

    // Prologue, 前言
    // 将fp压入栈中，保存fp的值
    printf(" # 将fp压栈，fp属于“被调用者保存”的寄存器，需要恢复原值\n");
    printf("  addi sp, sp, -8\n");
    printf("  sd fp, 0(sp)\n");
    // 将sp写入fp
    printf("  # 将sp的值写入fp\n");
    printf("  mv fp, sp\n");

    // 偏移量为实际变量所用的栈大小
    printf("  # sp腾出StackSize大小的栈空间\n");
    printf("  addi sp, sp, -%d\n", Prog->StackSize);

    printf("\n# =====程序主体===============\n");
    genStmt(Prog->Body);
    assert(Depth == 0);

    // Epilogue，后语
    // 输出return段标签
    printf("\n# =====程序结束===============\n");
    printf("# return段标签\n");
    printf(".L.return:\n");
    // 将fp的值改写回sp
    printf("  # 将fp的值写回sp\n");
    printf("  mv sp, fp\n");
    // 将最早fp保存的值弹栈，恢复fp。
    printf("  # 将最早fp保存的值弹栈，恢复fp和sp\n");
    printf("  ld fp, 0(sp)\n");
    printf("  addi sp, sp, 8\n");

    // 返回
    printf(" # 返回a0值给系统调用\n");
    printf("  ret\n");
}