
#
# input variables
#

## output dirs and objects
OUTPUT_DIR := build

ifndef RELEASE
OUTPUT_DIR := $(OUTPUT_DIR)/debug
GLOBAL_CFLAGS ?= -O0 -g
GLOBAL_CXXFLAGS ?= -O0 -g
else
OUTPUT_DIR := $(OUTPUT_DIR)/release
GLOBAL_CFLAGS ?= -Os
GLOBAL_CXXFLAGS ?= -Os
endif

OUTPUT_LIB = $(OUTPUT_DIR)/libwasm-interp.a
OUTPUT_EXEC = $(OUTPUT_DIR)/wasm-interp

## project root
GLOBAL_ROOT := .

## executables
GLOBAL_MKDIR ?= mkdir -p
GLOBAL_RM ?= rm -f

GLOBAL_CC ?= gcc
GLOBAL_CPP ?= g++
GLOBAL_AR ?= ar rcs

#
# build variables
#

# lib sources
# recursive source files
LIB_SRCS_DIRS += \
	src

# individual source files
LIB_SRCS_OBJS += \

# recursive includes
LIB_INCLUDES_DIRS += \
	src
	

# individual includes
LIB_INCLUDES_OBJS += \
	config \
	.


# exec sources
# recursive source files
EXEC_SRCS_DIRS += \
	exec

# individual source files
EXEC_SRCS_OBJS += \

#
# make lists
#

# search for sources
LIB_SRCS := \
	$(foreach dir,$(LIB_SRCS_DIRS),$(shell find $(GLOBAL_ROOT)/$(dir) -name '*.c*')) \
	$(addprefix $(GLOBAL_ROOT)/,$(LIB_SRCS_OBJS))

# search for sources
EXEC_SRCS := \
	$(foreach dir,$(EXEC_SRCS_DIRS),$(shell find $(GLOBAL_ROOT)/$(dir) -name '*.c*')) \
	$(addprefix $(GLOBAL_ROOT)/,$(EXEC_SRCS_OBJS))

# search for includes
LIB_INCLUDES := \
	$(foreach dir,$(LIB_INCLUDES_DIRS),$(shell find $(GLOBAL_ROOT)/$(dir) -type d)) \
	$(addprefix $(GLOBAL_ROOT)/,$(LIB_INCLUDES_OBJS))

# build object list to compile
LIB_OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(LIB_SRCS)))))

# build object list to compile
EXEC_OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(patsubst $(GLOBAL_ROOT)/%,$(OUTPUT_DIR)/%,$(EXEC_SRCS)))))

# build directory list to create
LIB_DIRS := $(sort $(dir $(LIB_OBJS)))

# build directory list to create
EXEC_DIRS := $(sort $(dir $(EXEC_OBJS)))

# build compiler include flag list
LIB_INPUT_CFLAGS := $(addprefix -I,$(LIB_INCLUDES))

BUILD_CXXFLAGS := $(GLOBAL_CXXFLAGS) $(LIB_INPUT_CFLAGS) $(CFLAGS) $(CPPFLAGS)
BUILD_CFLAGS := $(GLOBAL_CFLAGS) $(LIB_INPUT_CFLAGS) $(CFLAGS)

.DEFAULT_GOAL := all

# include sources dependencies
-include $(patsubst %.o,%.d,$(LIB_OBJS))
-include $(patsubst %.o,%.d,$(EXEC_OBJS))

# build cpp sources
$(OUTPUT_DIR)/%.o: $(GLOBAL_ROOT)/%.cpp
	$(GLOBAL_CPP) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(BUILD_CXXFLAGS) $< -c -o $@

# build cpp sources
$(OUTPUT_DIR)/%.o: $(GLOBAL_ROOT)/%.cc
	$(GLOBAL_CPP) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(BUILD_CXXFLAGS) $< -c -o $@

# build c sources
$(OUTPUT_DIR)/%.o: $(GLOBAL_ROOT)/%.c
	$(GLOBAL_CC) -MMD -MP -MF $(OUTPUT_DIR)/$*.d $(BUILD_CFLAGS) $< -c -o $@

$(OUTPUT_LIB): $(LIB_OBJS)
	$(GLOBAL_AR) $(OUTPUT_LIB) $(LIB_OBJS)

$(OUTPUT_EXEC): $(LIB_OBJS) $(EXEC_OBJS)
	$(GLOBAL_CPP) $(LIB_OBJS) $(EXEC_OBJS) -o $(OUTPUT_EXEC)

all: .prebuild $(OUTPUT_EXEC)

.prebuild:
	@$(GLOBAL_MKDIR) $(LIB_DIRS) $(EXEC_DIRS)

clean:
	$(GLOBAL_RM) -r $(OUTPUT_DIR)

.PHONY: all prebuild clean
