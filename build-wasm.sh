#!/bin/bash
args=("$@")

LLVM_BIN_DIR=../llvm-build/bin
BINARYEN_BIN_DIR=../binaryen/bin
WABT_BIN_DIR=./wabt/bin
AS_BIN=asc
WASM_DIR=./wasm

function build_wasm_cpp {
	$LLVM_BIN_DIR/clang -emit-llvm --target=wasm32 -Oz $WASM_DIR/cpp/hello.c -c -o $WASM_DIR/cpp/hello.bc
	$LLVM_BIN_DIR/llc -asm-verbose=false -o $WASM_DIR/cpp/hello.s $WASM_DIR/cpp/hello.bc
	$BINARYEN_BIN_DIR/s2wasm $WASM_DIR/cpp/hello.s > $WASM_DIR/cpp/hello.wast
	$WABT_BIN_DIR/wat2wasm $WASM_DIR/cpp/hello.wast -o $WASM_DIR/cpp/hello.wasm
	rm -f $WASM_DIR/cpp/hello.bc
	rm -f $WASM_DIR/cpp/hello.s
	rm -f $WASM_DIR/cpp/hello.wast
}

function build_wasm_as {
	$AS_BIN $WASM_DIR/as/hello.as -O --noRuntime --outFile $WASM_DIR/as/hello.wast
	$WABT_BIN_DIR/wat2wasm $WASM_DIR/as/hello.wast -o $WASM_DIR/as/hello.wasm
	rm -f $WASM_DIR/as/hello.wast
}


function run_wasm_cpp {
	$(pwd)/my-interp wasm/cpp/hello.wasm -E exportFunction
}

function run_wasm_as {
	$(pwd)/my-interp wasm/as/hello.wasm -E exportFunction
}


if [[ ${args[0]} = "build" ]] 
then
	if [[ ${args[1]} = "cpp" ]] 
	then
		build_wasm_cpp
	elif [[ ${args[1]} = "as" ]]
	then
		build_wasm_as
	fi
elif [[ ${args[0]} = "run" ]]
then
	if [[ ${args[1]} = "cpp" ]] 
	then
		run_wasm_cpp
	elif [[ ${args[1]} = "as" ]]
	then
		run_wasm_as
	fi
fi


