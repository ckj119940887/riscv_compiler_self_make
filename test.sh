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

# [2] 返回指定数值
assert 0 0

# [3] 支持+/-运算符
assert 34 34
assert 34 '12-34+56'

# [4] 支持空格
assert 41 ' 11 + 33-3 '

# [5] 支持* / ()运算符
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 17 '1-8/(2*2)+3*6'

# [6] 支持一元运算的+ -
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'
assert 48 '------12*+++++----++++++++++4'

# [7] 支持条件运算符
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'
assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'
assert 1 '5==2+3'
assert 0 '6==4+3'
assert 1 '0*9+5*2==4+4*(6/3)-2'

echo "ok"