# ==============================================================================
# VARIABLES

# BUILD DETAILS
CXX      			= g++
CXXFLAGS 			= -W -Wall
LFLAGS   			= -lm
BUILD    			= build

# RM
RM 					= rm

# MODE CONTROL FILES
MODE_FILE     		= $(BUILD)/mode.lock
DEBUG_MODE_FILE 	= $(BUILD)/debug.mode
RELEASE_MODE_FILE 	= $(BUILD)/release.mode

# COLORS
CLR_RED				= $(shell tput setaf 1)
CLR_GREEN			= $(shell tput setaf 2)
CLR_YELLOW			= $(shell tput setaf 3)
CLR_RESET			= $(shell tput sgr0)

# ==============================================================================
# TARGETS

TARGET = ascan

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(DEBUG_MODE_FILE) $(TARGET)

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(RELEASE_MODE_FILE) $(TARGET)

# ==============================================================================
# EXECUTABLE DETAILS

SRC = ascan.cpp 	\
	  options.cpp 	\
	  parser.cpp 	\
	  cfile.cpp 	\
	  align.cpp 	\
	  mfile.cpp 	\
	  common.cpp 	\
	  config.cpp

OBJ = $(addprefix $(BUILD)/, $(SRC:.cpp=.o))

$(TARGET): $(OBJ) # add any additional object files here #
	@printf "$(CLR_GREEN)Linking $@...\n$(CLR_RESET)"
	@printf "$(CXX) $(CXXFLAGS) -o $@ ... $(LFLAGS)\n"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# ==============================================================================
# CLEAN AND PHONY

clean:
	$(RM) -rf "$(TARGET)" "$(BUILD)"

.PHONY: default debug release clean

################################################################################
#         YOU PROBABLY DON'T WANT TO MODIFY ANYTHING BELOW THIS LINE		   #
################################################################################

# ==============================================================================
# MODE CONTROL

$(BUILD)/%.mode:
	@if [ ! -f "$@" ]; then 											\
		if [ -f $(MODE_FILE) ]; then 									\
			printf "$(CLR_GREEN)"; 										\
			printf "Switching to $* mode, rebuilding...\n"; 			\
			printf "$(CLR_RESET)"; 										\
			$(RM) -f "$(BUILD)"/*.mode;									\
			touch $(MODE_FILE) "$@"; 									\
			$(MAKE) -B $* --no-print-directory; 						\
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

$(BUILD)/%.d: %.cpp
	@echo "> Build dependencies for $<..."
	@mkdir -p $(BUILD)
	@$(CXX) $(CXXFLAGS) -MM $< | 										\
		sed 's/^\($*\)\.o/$$(BUILD)\/\1.o $$(BUILD)\/$*.d/' > $@;

ifneq ("$(MAKECMDGOALS)", "clean")
-include $(OBJ:%.o=%.d)
endif

