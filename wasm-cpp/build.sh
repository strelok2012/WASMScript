#!/bin/bash
export LLVM_BIN_DIR=../llvm-build/bin
export BINARYEN_BIN_DIR=../binaryen/bin
export WABT_DIR=../wabt/bin

$LLVM_BIN_DIR/clang -emit-llvm --target=wasm32 -Oz hello.c -c -o hello.bc

$LLVM_BIN_DIR/llc -asm-verbose=false -o hello.s hello.bc

$BINARYEN_BIN_DIR/s2wasm hello.s > hello.wast

$WABT_DIR/wat2wasm hello.wast -o hello.wasm

rm -f hello.bc
rm -f hello.s
rm -f hello.wast
