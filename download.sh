#!/bin/bash

# init and download submodules (wabt, binaryen)
git submodule update --init --recursive

# download LLVM-5 sources
wget http://releases.llvm.org/5.0.0/llvm-5.0.0.src.tar.xz
tar -xJf llvm-5.0.0.src.tar.xz llvm-5.0.0.src
mv llvm-5.0.0.src llvm
rm llvm-5.0.0.src.tar.xz

cd llvm/tools

# download clang sources
wget http://releases.llvm.org/5.0.0/cfe-5.0.0.src.tar.xz
tar -xJf cfe-5.0.0.src.tar.xz cfe-5.0.0.src
mv cfe-5.0.0.src clang
rm cfe-5.0.0.src.tar.xz

wget http://releases.llvm.org/5.0.0/lld-5.0.0.src.tar.xz
tar -xJf lld-5.0.0.src.tar.xz lld-5.0.0.src
mv lld-5.0.0.src lld
rm lld-5.0.0.src.tar.xz

cd -

WORKDIR=`pwd`
INSTALLDIR=`pwd`

rm -rf llvm-build
mkdir llvm-build
cd llvm-build

# For Debug build:
# cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=$INSTALLDIR -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Debug $WORKDIR/llvm
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=$INSTALLDIR -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly $WORKDIR/llvm 
