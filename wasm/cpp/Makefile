
#
# input variables
#

#EMSCRIPTEN := 1

## output dirs and objects
OUTPUT_DIR = build
OUTPUT_BYTECODE = $(OUTPUT_DIR)/cpp.bc
OUTPUT_LLVM_IR = $(OUTPUT_DIR)/cpp.s

OUTPUT_WAST = $(OUTPUT_DIR)/cpp.wast
OUTPUT_WASM = $(OUTPUT_DIR)/cpp.wasm

## project root
GLOBAL_ROOT := .

## default cflags
GLOBAL_CFLAGS ?= -Oz
GLOBAL_CXXFLAGS ?= -Oz

## executables
GLOBAL_MKDIR ?= mkdir -p
GLOBAL_RM ?= rm -f

EMSDK_DIR := ../../emsdk
EMSDK_CLANG_DIR := $(EMSDK_DIR)/clang/e1.37.22_64bit
EMSDK_EMSCRIPTEN_DIR := $(EMSDK_DIR)/emscripten/1.37.22

LLVM_BIN_DIR ?= ../../llvm-build/bin
BINARYEN_BIN_DIR ?= ../../binaryen/bin
WABT_BIN_DIR ?= ../../wabt/bin


#
# build variables
#

# recursive source files
SRCS_DIRS += \
	sources

# individual source files
SRCS_OBJS += \

# recursive includes
INCLUDES_DIRS += \
	sources

# individual includes
INCLUDES_OBJS += \

# search for sources
SRCS := \
	$(foreach dir,$(SRCS_DIRS),$(shell find $(GLOBAL_ROOT)/$(dir) -name '*.c*')) \
	$(addprefix $(GLOBAL_ROOT)/,$(SRCS_OBJS))

# search for includes
INCLUDES := \
	$(foreach dir,$(INCLUDES_DIRS),$(shell find $(GLOBAL_ROOT)/$(dir) -type d)) \
	$(addprefix $(GLOBAL_ROOT)/,$(INCLUDES_OBJS))


# build object list to compile
ifndef EMSCRIPTEN

OBJS := $(patsubst %.c,%.bc,$(patsubst %.cpp,%.bc,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(SRCS))))
LLVM_CFLAGS := -emit-llvm --target=wasm32 -fomit-frame-pointer -fno-threadsafe-statics -fno-stack-protector -fno-rtti -fno-exceptions -c
CC := $(LLVM_BIN_DIR)/clang
CPP := $(LLVM_BIN_DIR)/clang++
LINK := $(LLVM_BIN_DIR)/llvm-link
LLC := $(LLVM_BIN_DIR)/llc

else

OBJS := $(patsubst %.c,%.wasm,$(patsubst %.cpp,%.wasm,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(SRCS))))
LLVM_CFLAGS := 
CC = $(EMSDK_CLANG_DIR)/clang
CPP = $(EMSDK_CLANG_DIR)/clang++
LINK = $(EMSDK_CLANG_DIR)/llvm-link
LLC := $(EMSDK_CLANG_DIR)/llc

endif

# build directory list to create
DIRS := $(sort $(dir $(OBJS)))

# build compiler include flag list
INPUT_CFLAGS := $(addprefix -I,$(INCLUDES))

# build compiler flag list

CXXFLAGS := $(GLOBAL_CXXFLAGS) $(INPUT_CFLAGS) $(LLVM_CFLAGS) -std=c++14
CFLAGS := $(GLOBAL_CFLAGS) $(INPUT_CFLAGS) $(LLVM_CFLAGS)


.DEFAULT_GOAL := all

# include sources dependencies
-include $(patsubst %.bc,%.d,$(OBJS))

# build cpp sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.cpp
	$(CPP) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CXXFLAGS) $< -o $@

# build c sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.c
	$(CC) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CFLAGS) $< -o $@

# build llvm bytecode
$(OUTPUT_BYTECODE): $(OBJS)
	$(LINK) $(OBJS) -o $(OUTPUT_BYTECODE)

# decode llvm bytecode to text representation
$(OUTPUT_LLVM_IR): $(OUTPUT_BYTECODE)
	$(LLC) -asm-verbose=false -march=wasm32 -thread-model=single -O0 -o $@ $<

# encode llvm-ir to wasm text AST
$(OUTPUT_WAST): $(OUTPUT_LLVM_IR)
	$(BINARYEN_BIN_DIR)/s2wasm $< > $@

# encode wasm text AST to wasm binary form
$(OUTPUT_WASM): $(OUTPUT_WAST)
	$(WABT_BIN_DIR)/wat2wasm $< -o $@

all: .prebuild $(OUTPUT_WASM)

.prebuild:
	@echo $(OBJS)
	@$(GLOBAL_MKDIR) $(DIRS)

clean:
	$(GLOBAL_RM) -r $(OUTPUT_DIR)

.PHONY: all prebuild clean
