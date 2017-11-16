#!/bin/bash
export LLVM_BIN_DIR=../llvm-build/bin
export BINARYEN_BIN_DIR=../binaryen/bin
export WABT_DIR=../wabt/bin

rustc --target=wasm32-unknown-emscripten hello.rs

$LLVM_BIN_DIR/llc hello.ll -march=wasm32 -o hello.s

$BINARYEN_BIN_DIR/s2wasm hello.s > hello.wast

$WABT_DIR/wat2wasm hello.wast -o hello.wasm

