WASMScript - tools to build WebAssembly script modules

# Install (on Linux)

## Download dependencies

Use `download.sh` or

```sh
git submodule update --init --recursive
# Then, download latest LLVM and clang
```

## Build dependencies

LLVM build takes 15-40 minutes, and huge amount of memory.
Be careful with `make -jN`, it can consume more then 16Gb of memory.

LLVM (without download.sh):

```sh
mkdir llvm-build
cd llvm-build

cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=`pwd` -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly ../llvm 
make
```

LLVM (with download.sh):

```sh
cd llvm-build
make
```


WABT:

```sh
cd wabt
make
```

Binaryen:

```sh
cd binaryen
cmake .; make
```

## Build examples

C/C++

```sh
cd wasm/cpp
make
```
