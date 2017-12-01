
#
# input variables
#

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
OBJS := $(patsubst %.c,%.bc,$(patsubst %.cpp,%.bc,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(SRCS))))

# build directory list to create
DIRS := $(sort $(dir $(OBJS)))

# build compiler include flag list
INPUT_CFLAGS := $(addprefix -I,$(INCLUDES))

# build compiler flag list
LLVM_CFLAGS := -emit-llvm --target=wasm32

CXXFLAGS := $(GLOBAL_CXXFLAGS) $(INPUT_CFLAGS) $(LLVM_CFLAGS)
CFLAGS := $(GLOBAL_CFLAGS) $(INPUT_CFLAGS) $(LLVM_CFLAGS)


.DEFAULT_GOAL := all

# include sources dependencies
-include $(patsubst %.bc,%.d,$(OBJS))

# build cpp sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.cpp
	$(LLVM_BIN_DIR)/clang++ -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CXXFLAGS) $< -c -o $@

# build c sources
$(OUTPUT_DIR)/%.bc: $(GLOBAL_ROOT)/%.c
	$(LLVM_BIN_DIR)/clang -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(CFLAGS) $< -c -o $@

# build llvm bytecode
$(OUTPUT_BYTECODE): $(OBJS)
	$(LLVM_BIN_DIR)/llvm-link $(OBJS) -o $(OUTPUT_BYTECODE)

# decode llvm bytecode to text representation
$(OUTPUT_LLVM_IR): $(OUTPUT_BYTECODE)
	$(LLVM_BIN_DIR)/llc -asm-verbose=false -o $@ $<
	
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
