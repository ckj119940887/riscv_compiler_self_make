#!/bin/bash

assert()
{
    # 预期结果为参数1
    expected="$1"
    # 实际输入值
    input="$2"

    # 成功执行 || 之前的语句时将会短路exit
    ./build/rvcc "$input" > ./assembly/tmp.s || exit
    riscv64-unknown-linux-gnu-gcc -static ./assembly/tmp.s -o ./assembly/tmp
    qemu-riscv64 -L $RISCV/sysroot ./assembly/tmp
    # spike --isa=rv64gc $RISCV/riscv64-unknown-linux-gnu/bin/pk ./assembly/tmp

    # 实际结果
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
    fi
}

# 返回指定数值
assert 0 0

# 支持+/-运算符
assert 34 34
assert 34 '12-34+56'

# 支持空格
assert 41 ' 11 + 33-3 '

echo "ok"