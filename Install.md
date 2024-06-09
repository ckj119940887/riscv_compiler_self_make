# Ubuntu dependencies
```
sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev libglib2.0-dev ninja-build 
sudo apt-get install libusb-1.0-0-dev device-tree-compiler pkg-config cmake libpixman-1-dev
```

# riscv-gnu-toolchain源码包
```
wget https://mirror.iscas.ac.cn/plct/riscv-gnu-toolchain.20220725.tar.bz2
tar xvf riscv-gnu-toolchain.20220725.tar.bz2

cd riscv-gnu-toolchain
mkdir build
cd build

# 生成配置文件
../configure --prefix=$RISCV --with-arch=rv64gc --with-abi=lp64d

# 进行编译
make newlib -j4
make linux -j4
```

## riscv-pk
```
cd riscv-gnu-toolchain/pk
mkdir build && cd build
../configure --prefix=$RISCV --host=riscv64-unknown-elf
make
make install
```

## Spike
```
cd riscv-gnu-toolchain/spike
mkdir build && cd build
../configure --prefix=$RISCV
make -j4
make install
```

# RVV-LLVM
```
git clone https://github.com/isrc-cas/rvv-llvm.git
cd rvv-llvm
mkdir build && cd build
cmake -DLLVM_TARGETS_TO_BUILD="RISCV" -DLLVM_ENABLE_PROJECTS=clang -DMAKE_INSTALL_PREFIX=/home/llvm_tools -G "Unix Makefiles" ../llvm
make -j4
make install
```

# QEMU
```
wget https://mirror.iscas.ac.cn/plct/qemu.20201124.tar.bz2
tar xvf qemu.20201124.tar.bz2
cd qemu
mkdir build && cd build
../configure --target-list=riscv64-linux-user,riscv64-softmmu --prefix=/home/qemu
make -j4
make install
```

# test

## hello.c
```
#include <stdio.h>
int main()
{
    printf("hello world!\n"); 
    return 0;
}
```

## spike-test
```
riscv64-unknown-elf-gcc hello.c -o hello
spike pk hello
```

## qemu-test
```
riscv64-unknown-elf-gcc hello.c -o hello
qemu-riscv64 hello
```