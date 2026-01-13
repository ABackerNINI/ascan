PROJECT_ROOT := $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# PROJECT

PROJECT = __ASCAN::PROJECT_NAME__

# OPTIONS

__ASCAN::OPTIONS_SECTION__

# DIRECTORIES

# The directory of source files. Must be subdirectory of the project root. Can be set to ".".
SRC_DIR = __ASCAN::SRC_DIR__
# Where to put object files. Must be subdirectory of the project root. Can be set to ".".
BLD_DIR = __ASCAN::BLD_DIR__
# Where to put the final binary. Must be subdirectory of the project root. Can be set to ".".
BIN_DIR = __ASCAN::BIN_DIR__
# Extra parameters to identify the build configuration.
# This is used to create a unique object directory for each build configuration.
EXTRA_PARAMS =
PARAMS_SIGNATURE = $(shell echo "a$(EXTRA_PARAMS)b" | md5sum | cut -c1-12)
# Where to put object files for each build configuration. Must be subdirectory of the project root. Can be set to ".".
OBJ_DIR = $(BLD_DIR)/$(CONFIG)/.__ASCAN::OBJ_DIR_CC_CXX_STD__.$(PARAMS_SIGNATURE)
# This file is used to store the configuration of the last successful build, so that
# when the configuration changes, we can rebuild the target.
CONFIG_FILE = $(BLD_DIR)/$(CONFIG).__ASCAN::CONFIG_FILE_CC_CXX_STD__.$(PARAMS_SIGNATURE).ascan.conf

# CHECK DIRECTORIES

# Check that directories do not contain spaces.
SPACE_CHECKS = $(words $(SRC_DIR)) $(words $(BLD_DIR)) \
		 	 $(words $(OBJ_DIR)) $(words $(BIN_DIR))
ifneq ($(filter-out 1, $(SPACE_CHECKS)),)
$(error "SRC_DIR, BLD_DIR, OBJ_DIR, BIN_DIR must be set to a directory. Please check your Makefile.")
endif

# Check that directories are subdirectories of the project root.
SUB_DIR_CHECKS = $(shell echo $(abspath $(SRC_DIR)) | grep -q "^$(PROJECT_ROOT)" || echo 0) \
				 $(shell echo $(abspath $(BLD_DIR)) | grep -q "^$(PROJECT_ROOT)" || echo 0) \
				 $(shell echo $(abspath $(OBJ_DIR)) | grep -q "^$(PROJECT_ROOT)" || echo 0) \
				 $(shell echo $(abspath $(BIN_DIR)) | grep -q "^$(PROJECT_ROOT)" || echo 0)
ifeq ($(filter 0, $(SUB_DIR_CHECKS)),0)
$(error "SRC_DIR, BLD_DIR, OBJ_DIR, BIN_DIR must be subdirectories of the project root. Please check your Makefile.")
endif

# BUILD DETAILS

__ASCAN::C_CXX_FLAGS__
LDFLAGS = __ASCAN::LD_FLAGS__

__ASCAN::TARGETS__

ifeq ($(CONFIG),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG),release)
    CXXFLAGS += -flto=4 -O3 -march=native -DNDEBUG
endif

# SOURCES

__ASCAN::SOURCES_SECTION__

# OBJECTS

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/$(SRC_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# TARGETS

__ASCAN::TARGETS_SECTION__

# RULES

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BLD_DIR)/%.ascan.conf:
	@mkdir -p $(@D)
	@rm -f $(@D)/*.ascan.conf
	@touch $@

# DEPENDENCIES

-include $(DEPS)

# PHONY TARGETS

debug:
	@$(MAKE) CONFIG=debug --no-print-directory

release:
	@$(MAKE) CONFIG=release --no-print-directory

test:
	@$(MAKE) CONFIG=test --no-print-directory

all: debug release

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET) $(OBJS) $(DEPS) $(BLD_DIR)/*.ascan.conf

	@#! CAUTION: rm -rf command
	@if [ ! "$(BLD_DIR)" = "." ] && [ ! "$(OBJ_DIR)" = "/"  ]; then \
		rm -rf "$(BLD_DIR)"; \
	fi

	@#! CAUTION: rm -rf command
	@if [ ! "$(BIN_DIR)" = "." ] && [ ! "$(BIN_DIR)" = "/"  ]; then \
		rm -rf "$(BIN_DIR)"; \
	fi

.PHONY: debug release test all clean

# CHECK BUILD PARAMS

RED		  = $(shell tput setaf 1)
GREEN	  = $(shell tput setaf 2)
YELLOW	  = $(shell tput setaf 3)
BOLD 	  = $(shell tput bold)
UNDERLINE = $(shell tput smul)
RESET	  = $(shell tput sgr0)

define save_build_params
	@echo "PROJECT=$(PROJECT)" > "$(1)"
	@echo "CONFIG=$(CONFIG)" >> "$(1)"
	@echo "CC=$(CC)" >> "$(1)"
	@echo "CXX=$(CXX)" >> "$(1)"
	@echo "STD=$(STD)" >> "$(1)"
	@echo "CSTD=$(CSTD)" >> "$(1)"
	@echo "CXXSTD=$(CXXSTD)" >> "$(1)"
	@echo "SRC_DIR=$(SRC_DIR)" >> "$(1)"
	@echo "BLD_DIR=$(BLD_DIR)" >> "$(1)"
	@echo "BIN_DIR=$(BIN_DIR)" >> "$(1)"
	@echo "EXTRA_PARAMS=$(EXTRA_PARAMS)" >> "$(1)"
	@echo "OBJ_DIR=$(OBJ_DIR)" >> "$(1)"
	@echo "CONFIG_FILE=$(CONFIG_FILE)" >> "$(1)"
	@echo "CXXFLAGS=$(CXXFLAGS)" >> "$(1)"
	@echo "LDFLAGS=$(LDFLAGS)" >> "$(1)"
endef

define check_build_params
	$(call save_build_params,$(OBJ_DIR)/temp.txt)

	@if [ -f $(OBJ_DIR)/build_params.txt ]; then \
		if ! diff -q "$(OBJ_DIR)/build_params.txt" "$(OBJ_DIR)/temp.txt" >/dev/null 2>&1 ; then \
			echo "$(RED)$(BOLD)"; \
			echo -n "WARNING: Build parameters mismatch, most likely due to Makefile changes, or a hash collision, "; \
			echo "you may need a 'make clean'."; \
			echo "$(RESET)"; \
			echo "Diff of build parameters (last vs current):"; \
			diff --ignore-space-change --color --minimal "$(OBJ_DIR)/build_params.txt" "$(OBJ_DIR)/temp.txt"; \
			echo ""; \
		fi \
	fi

	@mv -f "$(OBJ_DIR)/temp.txt" "$(OBJ_DIR)/build_params.txt"
endef
