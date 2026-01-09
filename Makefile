PROJECT = ascan

# OPTIONS

CONFIG 		?= debug

CXX 		?= g++
STD       	?= c++17

# SOURCES

SRC_DIR = src
# SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS = ascan.cpp options.cpp parser.cpp cfile.cpp align.cpp mfile.cpp common.cpp config.cpp
SRCS := $(SRCS:%.cpp=$(SRC_DIR)/%.cpp)

# CONFIGURATIONS

PARAMS =
PARAMS_SIGNATURE = $(shell echo "1$(PARAMS)1" | md5sum | cut -c1-12)
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/$(CONFIG)/$(CXX).$(STD).$(PARAMS_SIGNATURE)
BIN_DIR = bin

CXXFLAGS = -Wall -Wextra -std=$(STD) -I. -I./libs/ \
		   -Wno-unused-parameter
LDFLAGS  = -L./libs -lfmt

TARGET = $(PROJECT)

ifeq ($(CONFIG),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG),release)
    CXXFLAGS += -flto=4 -O3 -march=native -DNDEBUG
else ifeq ($(CONFIG),test)
    CXXFLAGS += -g -O0 -DTEST -Igoogletest/include
    TARGET = $(PROJECT)_test
    LDFLAGS += -Lgoogletest/lib -lgtest -lgtest_main -lpthread
endif

# OBJECTS

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# MAIN TARGET

$(BIN_DIR)/$(TARGET): $(OBJS) $(BUILD_DIR)/$(CONFIG).mode
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	$(call check_build_params)

# COMPILE RULE

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(CONFIG).mode:
	@mkdir -p $(dir $@)
	@rm -f $(dir $@)/*.mode
	@touch $@

# INCLUDE DEPENDENCIES

-include $(DEPS)

# PHONY TARGETS

.PHONY: debug release test clean all

debug:
	@$(MAKE) CONFIG=debug --no-print-directory

release:
	@$(MAKE) CONFIG=release --no-print-directory

test:
	@$(MAKE) CONFIG=test --no-print-directory

all: debug release

clean:
	rm -rf $(BUILD_DIR)/ $(BIN_DIR)/

# BUILD PARAMS

RED = \033[0;31m
RESET = \033[0m

define save_build_params
	@echo "PROJECT=$(PROJECT)" > "$(1)"
	@echo "CONFIG=$(CONFIG)" >> "$(1)"
	@echo "CXX=$(CXX)" >> "$(1)"
	@echo "STD=$(STD)" >> "$(1)"
	@echo "CXXFLAGS=$(CXXFLAGS)" >> "$(1)"
endef

define check_build_params
	$(call save_build_params,$(OBJ_DIR)/temp.txt)

	@if [ -f $(OBJ_DIR)/build_params.txt ]; then \
		if ! diff -q "$(OBJ_DIR)/build_params.txt" "$(OBJ_DIR)/temp.txt" >/dev/null 2>&1 ; then \
			echo "$(RED)"; \
			echo "WARNING: Build params mismatch, most likely due to Makefile changes, or barely hash collision, try 'make clean' first."; \
			echo "$(RESET)"; \
			echo "Diff of build params:"; \
			diff --ignore-space-change --color --minimal "$(OBJ_DIR)/build_params.txt" "$(OBJ_DIR)/temp.txt"; \
			echo ""; \
		fi \
	fi

	$(call save_build_params,$(OBJ_DIR)/build_params.txt)
endef
