# BUILD DETAILS

CXX      = g++
CXXFLAGS = -W -Wall -DDISABLE_WRITE=0
LFLAGS   = -lm
BUILD    = build

# TARGETS

TARGET1  = ascan
TARGET2  = opt_parser

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode $(TARGET1) $(TARGET2)

release: CXXFLAGS += -O3 # -DNDEBUG=1
release: $(BUILD)/release.mode $(TARGET1) $(TARGET2)

# EXECUTABLES

OBJ1     = ascan.o options.o parser.o cfile.o align.o mfile.o common.o config.o
OBJ2     = opt_parser.o

OBJ1BD   = $(OBJ1:%=$(BUILD)/%)
OBJ2BD   = $(OBJ2:%=$(BUILD)/%)

$(TARGET1): $(OBJ1BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

$(TARGET2): $(OBJ2BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.cpp
	@mkdir -p "$(BUILD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# MODE CONTROL

$(BUILD)/%.mode:
	@if [ ! -f "$@" ]; then \
		if [ -d "$(BUILD)" ]; then \
			echo "Switching to $* mode..."; \
			$(MAKE) clean; \
		fi; \
		mkdir -p "$(BUILD)"; \
		touch "$@"; \
	fi

# CLEAN

clean:
	rm -rf "$(TARGET1)" "$(TARGET2)" "$(BUILD)"

# PHONY

.PHONY: default debug release clean

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@rm -f "$@"
	@$(CXX) -MM *.cpp | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' >> $@

include $(BUILD)/depend.mk

