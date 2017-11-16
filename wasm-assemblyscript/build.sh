#!/bin/bash
export WABT_DIR=../wabt/bin
asc hello.as -O --noRuntime --outFile hello.wast
$WABT_DIR/wat2wasm hello.wast -o hello.wasm
