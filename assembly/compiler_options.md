# 使用linux gcc交叉编译工具链
```
riscv64-unknown-linux-gnu-gcc -static tmp.s -o tmp
qmeu-riscv64 -L $RISCV/sysroot tmp

想查看程序执行的结果
需要使用 echo $? 来查看程序最后执行的结果
```