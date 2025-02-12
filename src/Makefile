# BUILD DETAILS

CXX      = g++
CXXFLAGS = -W -Wall -g
LFLAGS   = -lm
BUILD    = build

# TARGETS

TARGET1  = ascan
TARGET2  = tlexer
TARGET3  = tparser

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode $(TARGET1)

release: CXXFLAGS += -O3 # -DNDEBUG=1
release: $(BUILD)/release.mode $(TARGET1)

# EXECUTABLE DETAILS

OBJ1     = ascan.o options.o parser.o cfile.o align.o mfile.o common.o config.o
OBJ2     = tlexer.o
OBJ3     = tparser.o tlexer.o

OBJ1BD   = $(OBJ1:%=$(BUILD)/%)
OBJ2BD   = $(OBJ2:%=$(BUILD)/%)
OBJ3BD   = $(OBJ3:%=$(BUILD)/%)

$(TARGET1): $(OBJ1BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

$(TARGET2): $(OBJ2BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

$(TARGET3): $(OBJ3BD)
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

# CLEAN UP

clean:
	rm -rf "$(TARGET1)" "$(TARGET2)" "$(TARGET3)" "$(BUILD)"

# PHONY

.PHONY: default debug release clean

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@rm -f "$@"
	@$(CXX) $(CXXFLAGS) -MM *.cpp | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' >> $@

include $(BUILD)/depend.mk

