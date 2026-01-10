PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# PROJECT

PROJECT = ascan

# OPTIONS

CONFIG 		?= debug

CXX 		?= g++
STD       	?= c++17

# DIRECTORIES

PARAMS =
PARAMS_SIGNATURE = $(shell echo "1$(PARAMS)1" | md5sum | cut -c1-12)
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
OBJ_DIR = $(BUILD_DIR)/$(CONFIG)/$(CXX).$(STD).$(PARAMS_SIGNATURE)
MODE_FILE = $(BUILD_DIR)/$(CONFIG).mk.mode

# CHECK DIRECTORIES

# Check that directories do not contain spaces
DIR_CHECKS = $(words $(SRC_DIR)) $(words $(BUILD_DIR)) \
		 	 $(words $(OBJ_DIR)) $(words $(BIN_DIR))
ifneq ($(filter-out 1, $(DIR_CHECKS)),)
$(error "SRC_DIR, BUILD_DIR, OBJ_DIR, BIN_DIR must be set to a directory. Please check your Makefile.")
endif

# Check that directories are subdirectories of PROJECT_ROOT
PREFIX = $(abspath $(PROJECT_ROOT))
SUB_DIR_CHECKS = $(shell echo $(abspath $(SRC_DIR))   | grep -q "^$(PREFIX)" || echo 0) \
				 $(shell echo $(abspath $(BUILD_DIR)) | grep -q "^$(PREFIX)" || echo 0) \
				 $(shell echo $(abspath $(OBJ_DIR))   | grep -q "^$(PREFIX)" || echo 0) \
				 $(shell echo $(abspath $(BIN_DIR))   | grep -q "^$(PREFIX)" || echo 0)
ifeq ($(filter 0, $(SUB_DIR_CHECKS)),0)
$(error "SRC_DIR, BUILD_DIR, OBJ_DIR, BIN_DIR must be subdirectories of the project root. Please check your Makefile.")
endif

# BUILD DETAILS

CXXFLAGS = -Wall -Wextra -std=$(STD) -I. -I./libs/ \
		   -Wno-unused-parameter
LDFLAGS  = -L./libs -lfmt

TARGET = $(BIN_DIR)/$(PROJECT)

ifeq ($(CONFIG),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG),release)
    CXXFLAGS += -flto=4 -O3 -march=native -DNDEBUG
else ifeq ($(CONFIG),test)
    CXXFLAGS += -g -O0 -DTEST -Igoogletest/include
    TARGET = $(PROJECT)_test
    LDFLAGS += -Lgoogletest/lib -lgtest -lgtest_main -lpthread
endif

# SOURCES

# SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS := ascan.cpp options.cpp parser.cpp cfile.cpp align.cpp mfile.cpp common.cpp config.cpp
SRCS := $(SRCS:%.cpp=$(SRC_DIR)/%.cpp)

# OBJECTS

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# MAIN TARGET

$(TARGET): $(OBJS) $(MODE_FILE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	$(call check_build_params)

# COMPILE RULE

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.mk.mode:
	@mkdir -p $(@D)
	@rm -f $(@D)/*.mk.mode
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
	@rm -f $(TARGET) $(OBJS) $(DEPS) $(BUILD_DIR)/*.mk.mode

	@if [ ! "$(BUILD_DIR)" = "." ]; then \
		rm -rf $(BUILD_DIR); \
	fi

	@if [ ! "$(BIN_DIR)" = "." ]; then \
		rm -rf $(BIN_DIR); \
	fi

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

# Directory string should not be empty or contains spaces.
define check_dirs
	@echo "Checking for empty paths..." \
	@if [ "$(SRC_DIR)" = "" ] || \
	    [ "$(BUILD_DIR)" = "" ] || \
		[ "$(OBJ_DIR)" = "" ] || \
		[ "$(BIN_DIR)" = "" ]; then \
		echo "$(RED)Error: SRC_DIR, BUILD_DIR, OBJ_DIR, and BIN_DIR must be set$(RESET)"; \
		exit 1; \
	fi
	@echo "Checking for spaces in paths..."
	@if [ -n "$$(echo "$(SRC_DIR)" | grep ' ')" ] || \
		[ -n "$$(echo "$(BUILD_DIR)" | grep ' ')" ] || \
		[ -n "$$(echo "$(OBJ_DIR)" | grep ' ')" ] || \
		[ -n "$$(echo "$(BIN_DIR)" | grep ' ')" ]; then \
		echo "$(RED)Error: SRC_DIR, BUILD_DIR, OBJ_DIR, and BIN_DIR must not contain spaces$(RESET)"; \
		exit 1; \
	fi
endef

test_makefile:
