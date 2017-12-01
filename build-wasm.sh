#!/bin/bash
args=("$@")

LLVM_BIN_DIR=../llvm/bin
BINARYEN_BIN_DIR=../binaryen/bin
WABT_BIN_DIR=./wabt/bin
AS_BIN=asc
WASM_DIR=./wasm

function build_wasm_c {
	$LLVM_BIN_DIR/clang -emit-llvm --target=wasm32 -Oz $WASM_DIR/c/hello.c -c -o $WASM_DIR/c/hello.bc
	$LLVM_BIN_DIR/clang -emit-llvm --target=wasm32 -Oz $WASM_DIR/c/hello-2.c -c -o $WASM_DIR/c/hello-2.bc
	$LLVM_BIN_DIR/llvm-link $WASM_DIR/c/hello.bc $WASM_DIR/c/hello-2.bc -o $WASM_DIR/c/hello-c.bc
	$LLVM_BIN_DIR/llc -asm-verbose=false -o $WASM_DIR/c/hello-c.s $WASM_DIR/c/hello-c.bc
	$BINARYEN_BIN_DIR/s2wasm $WASM_DIR/c/hello-c.s > $WASM_DIR/c/hello-c.wast
	$WABT_BIN_DIR/wat2wasm $WASM_DIR/c/hello-c.wast -o $WASM_DIR/c/hello-c.wasm
	rm -f $WASM_DIR/c/*.bc
	rm -f $WASM_DIR/c/*.s
	rm -f $WASM_DIR/c/*.wast
}

function build_wasm_as {
	$AS_BIN $WASM_DIR/as/hello.as -O --noRuntime --outFile $WASM_DIR/as/hello.wast
	$WABT_BIN_DIR/wat2wasm $WASM_DIR/as/hello.wast -o $WASM_DIR/as/hello-as.wasm
	rm -f $WASM_DIR/as/hello.wast
}

function build_wasm_rust {
	rustc $WASM_DIR/rust/hello.rs --target=wasm32-unknown-unknown -C panic=abort -o $WASM_DIR/rust/hello-rust.wasm
	wasm-gc $WASM_DIR/rust/hello-rust.wasm $WASM_DIR/rust/hello-rust.wasm
	#$WABT_BIN_DIR/wasm2wat $WASM_DIR/rust/hello.wasm -o $WASM_DIR/rust/hello-rust.wast
}


function run_wasm_c {
	echo -e "\033[32mRunning C...\033[0m"
	$(pwd)/my-interp wasm/c/hello-c.wasm -E export_function
}

function run_wasm_as {
	echo -e "\033[32mRunning AssemblyScript...\033[0m"
	$(pwd)/my-interp wasm/as/hello-as.wasm -E export_function
}

function run_wasm_rust {
	echo -e "\033[32mRunning Rust...\033[0m"
	$(pwd)/my-interp wasm/rust/hello-rust.wasm -E export_function
}

if [[ ${args[0]} = "build" ]] 
then
	if [[ ${args[1]} = "c" ]] 
	then
		build_wasm_c
	elif [[ ${args[1]} = "as" ]]
	then
		build_wasm_as
	elif [[ ${args[1]} = "rust" ]]
	then
		build_wasm_rust
	elif [[ ${args[1]} = "all" ]]
	then
		build_wasm_c
		build_wasm_as
		build_wasm_rust
	fi
elif [[ ${args[0]} = "run" ]]
then
	if [[ ${args[1]} = "c" ]] 
	then
		run_wasm_c
	elif [[ ${args[1]} = "as" ]]
	then
		run_wasm_as
	elif [[ ${args[1]} = "rust" ]]
	then
		run_wasm_rust
	elif [[ ${args[1]} = "all" ]]
	then
		run_wasm_c
		run_wasm_as
		run_wasm_rust
	fi
fi


