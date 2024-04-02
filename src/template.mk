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
$(ASCAN_TARGETS)

# ==============================================================================
# RULES

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode $(TARGET)

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(BUILD)/release.mode $(TARGET)

# ==============================================================================
# EXECUTABLE DETAILS

SRCS =

OBJS = $(addprefix $(BUILD)/, $(notdir $(SRCS:.cpp=.o)))

$(TARGET): $(OBJS) # add any additional object files here #
	@printf "$(CLR_GREEN)Linking $@...\n$(CLR_RESET)"
	@printf "$(CXX) $(CXXFLAGS) -o $@ ... $(LFLAGS)\n"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# ==============================================================================
# CLEAN, PHONY, SECONDARY

clean:
	$(RM) -rf "$(BUILD)" "$(TARGET)"

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
