# ==============================================================================
# VARIABLES

# BUILD DETAILS
CXX      			= g++
CXXFLAGS 			= -W -Wall
LFLAGS   			= -lm
BUILD    			= build

# RM
RM 					= rm

# COLORS
CLR_RED				= $(shell tput setaf 1)
CLR_GREEN			= $(shell tput setaf 2)
CLR_YELLOW			= $(shell tput setaf 3)
CLR_RESET			= $(shell tput sgr0)

# TARGETS
__ASCAN_BEGIN__
python3 targets.py targets_def $(ascan.targets)
__ASCAN_END__

# ==============================================================================
# RULES

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(BUILD)/release.mode __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__

# ==============================================================================
# EXECUTABLE DETAILS

$(__ASCAN_SUB_TEMPLATE_TARGETS_DETAILS_BEGIN__)
SRCS$(__ASCAN_TARGET_INDEX__) = $(__ASCAN_TARGET_SOURCES__)

OBJS$(__ASCAN_TARGET_INDEX__) = $(addprefix $(BUILD)/, $(notdir $(SRCS$(__ASCAN_TARGET_INDEX__):.cpp=.o)))

$(TARGET$(__ASCAN_TARGET_INDEX__)): $(OBJS) # add any additional object files here #
	@printf "$(CLR_GREEN)Linking $@...\n$(CLR_RESET)"
	@printf "$(CXX) $(CXXFLAGS) -o $@ ... $(LFLAGS)\n"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)
$(__ASCAN_SUB_TEMPLATE_TARGETS_DETAILS_END__)

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
python3 executable.py $(ascan.targets) ; $(ascan.sources)
__ASCAN_END__

# ==============================================================================
# CLEAN, PHONY, SECONDARY

clean:
	$(RM) -rf "$(BUILD)" __ASCAN_BEGIN__ python3 targets.py list $(ascan.targets) __ASCAN_END__

.PHONY: default 	\
		debug 		\
		release 	\
		clean

.SECONDARY: $(BUILD)/%.d $(BUILD)/%.o

################################################################################
#         YOU PROBABLY DON'T WANT TO MODIFY ANYTHING BELOW THIS LINE		   #
################################################################################

# ==============================================================================
# MODE CONTROL

MODE_FILE = $(BUILD)/mode.lock

$(BUILD)/%.mode:
	@if [ ! -f "$@" ]; then 											\
		if [ -f $(MODE_FILE) ]; then 									\
			printf "$(CLR_GREEN)"; 										\
			printf "Switching to $* mode, rebuilding...\n"; 			\
			printf "$(CLR_RESET)"; 										\
			$(RM) -f "$(BUILD)"/*.mode;									\
			touch $(MODE_FILE) "$@"; 									\
			$(MAKE) --no-print-directory -s depend; 					\
			$(MAKE) --no-print-directory -B $* MAKE_NODEPS=1; 			\
		else 															\
			mkdir -p $(BUILD); 											\
			touch $(MODE_FILE) "$@"; 									\
		fi; 															\
	fi

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
