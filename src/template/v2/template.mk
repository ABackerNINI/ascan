# ==============================================================================
# VARIABLES

# BUILD DETAILS
CXX      			= g++
CXXFLAGS 			= -W -Wall
LFLAGS   			= -lm
BUILD    			= build

# RM
RM 					= rm

__ASCAN_BEGIN__
ascan-if --color and no --no-color:
# COLORS
CLR_RED				= $(shell tput setaf 1)
CLR_GREEN			= $(shell tput setaf 2)
CLR_YELLOW			= $(shell tput setaf 3)
CLR_RESET			= $(shell tput sgr0)
ascan-fi
__ASCAN_END__

# TARGETS
__ASCAN_BEGIN__
# TARGET1 = ascan
# TARGET2 = xxx
python3 targets.py targets_def $(ascan.targets)
__ASCAN_END__

# ==============================================================================
# RULES
__ASCAN_BEGIN__
ascan-if --mode-control and no --no-mode-control:
default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(BUILD)/release.mode __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__
ascan-else:
default: __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__
ascan-fi
__ASCAN_END__

# ==============================================================================
# EXECUTABLE DETAILS

__ASCAN_BEGIN__
# SRCS = ascan.cpp 	\
# 	   options.cpp 	\
# 	   parser.cpp 	\
# 	   cfile.cpp 	\
# 	   align.cpp 	\
# 	   mfile.cpp 	\
# 	   common.cpp 	\
# 	   config.cpp
#
# OBJS = $(addprefix $(BUILD)/, $(notdir $(SRCS:.cpp=.o)))
#
# $(TARGET): $(OBJS) # add any additional object files here #
# 	@printf "$(CLR_GREEN)Linking $@...\n$(CLR_RESET)"
# 	@printf "$(CXX) $(CXXFLAGS) -o $@ ... $(LFLAGS)\n"
# 	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)
python3 executable.py $(ascan.args) ; $(ascan.targets) ; $(ascan.sources)
__ASCAN_END__

# ==============================================================================
# CLEAN, PHONY, SECONDARY

clean:
	$(RM) -rf "$(BUILD)" __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__

__ASCAN_BEGIN__
ascan-if --mode-control and no --no-mode-control:
.PHONY: default 	\
		debug 		\
		release 	\
		clean
ascan-else:
.PHONY: default clean
ascan-fi
__ASCAN_END__

.SECONDARY: $(BUILD)/%.d $(BUILD)/%.o

################################################################################
#         YOU PROBABLY DON'T WANT TO MODIFY ANYTHING BELOW THIS LINE		   #
################################################################################

__ASCAN_BEGIN__
ascan-if --mode-control and no --no-mode-control:
# ==============================================================================
# MODE CONTROL

MODE_FILE = $(BUILD)/mode.lock

$(BUILD)/%.mode:
	@if [ ! -f "$@" ]; then 											\
		if [ -f $(MODE_FILE) ]; then 									\
		ascan-if --color and no --no-color:
			printf "$(CLR_GREEN)"; 										\
		ascan-fi
			printf "Switching to $* mode, rebuilding...\n"; 			\
		ascan-if --color and no --no-color:
			printf "$(CLR_RESET)"; 										\
		ascan-fi
			$(RM) -f "$(BUILD)"/*.mode;									\
			touch $(MODE_FILE) "$@"; 									\
			$(MAKE) --no-print-directory -s depend; 					\
			$(MAKE) --no-print-directory -B $* MAKE_NODEPS=1; 			\
		else 															\
			mkdir -p $(BUILD); 											\
			touch $(MODE_FILE) "$@"; 									\
		fi; 															\
	fi
ascan-fi
__ASCAN_END__

# ==============================================================================
# COMPILE OBJECT FILES

$(BUILD)/%.o: %.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# ==============================================================================
# DEPENDENCIES

DEPS = $(OBJ:%.o=%.d)

.PHONY:
depend: $(DEPS)

ifndef MAKE_NODEPS
$(BUILD)/%.d: %.cpp
	@echo "> Build dependencies for $<..."
	@mkdir -p $(BUILD)
	@$(CXX) $(CXXFLAGS) -MM $< | 										\
		sed 's/^\($*\)\.o/$$(BUILD)\/\1.o $$(BUILD)\/$*.d/' > $@;

ifneq ("$(MAKECMDGOALS)", "clean")
-include $(DEPS)
endif
endif
