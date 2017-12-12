
#
# input variables
#

## output dirs and objects
OUTPUT_DIR = build
OUTPUT_BYTECODE = $(OUTPUT_DIR)/cpp.bc
OUTPUT_LLVM_IR = $(OUTPUT_DIR)/cpp.s

OUTPUT_WAST = $(OUTPUT_DIR)/cpp.wast
OUTPUT_WASM = $(OUTPUT_DIR)/cpp.wasm
OUTPUT_WAST_FINAL = $(OUTPUT_DIR)/cpp.wast.final

## project root
GLOBAL_ROOT := .

## default cflags
GLOBAL_CFLAGS ?= -Os
GLOBAL_CXXFLAGS ?= -Os

## executables
GLOBAL_MKDIR ?= mkdir -p
GLOBAL_RM ?= rm -f

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

OBJS := $(patsubst %.c,%.bc,$(patsubst %.cpp,%.bc,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(SRCS))))

CC := $(LLVM_BIN_DIR)/clang
CPP := $(LLVM_BIN_DIR)/clang++
LINK := $(LLVM_BIN_DIR)/llvm-link
LLC := $(LLVM_BIN_DIR)/llc

# build directory list to create
DIRS := $(sort $(dir $(OBJS)))

# build compiler include flag list
INPUT_CFLAGS := $(addprefix -I,$(INCLUDES)) \
	-emit-llvm --target=wasm32 -fomit-frame-pointer -fno-threadsafe-statics -fno-stack-protector -fno-rtti -fno-exceptions

# build compiler flag list

CXXFLAGS := $(GLOBAL_CXXFLAGS) $(INPUT_CFLAGS) -std=c++14
CFLAGS := $(GLOBAL_CFLAGS) $(INPUT_CFLAGS)


.DEFAULT_GOAL := all

# include sources dependencies
-include $(patsubst %.bc,%.d,$(OBJS))

# build cpp sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.cpp
	$(CPP) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CXXFLAGS) -c $< -o $@

# build c sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.c
	$(CC) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CFLAGS) -c $< -o $@

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
	$(BINARYEN_BIN_DIR)/wasm-as $< -o $@

# encode wasm text AST to wasm binary form
$(OUTPUT_WAST_FINAL): $(OUTPUT_WASM)
	$(WABT_BIN_DIR)/wasm2wat $< -o $@

all: .prebuild $(OUTPUT_WAST_FINAL)

.prebuild:
	@$(GLOBAL_MKDIR) $(DIRS)

clean:
	$(GLOBAL_RM) -r $(OUTPUT_DIR)

.PHONY: all prebuild clean
