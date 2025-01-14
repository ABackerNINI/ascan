# BUILD DETAILS

CXX      = g++
CXXFLAGS = -W -Wall -g
LFLAGS   = -lm
BUILD    = build

# BUILD EXECUTABLE

TARGET1  = ascan
TARGET2  = opt_parser

all: $(TARGET1) $(TARGET2)

# CLEAN UP

clean:
	rm -rf "$(TARGET1)" "$(TARGET2)" "$(BUILD)"

# EXECUTABLE 1

OBJ1     = ascan.o options.o parser.o cfile.o align.o mfile.o common.o config.o
OBJ1BD   = $(OBJ1:%=$(BUILD)/%)

$(TARGET1): $(OBJ1BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# EXECUTABLE 2

OBJ2     = opt_parser.o
OBJ2BD   = $(OBJ2:%=$(BUILD)/%)

$(TARGET2): $(OBJ2BD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.cpp
	@mkdir -p "$(BUILD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# PHONY

.PHONY: all clean

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@rm -f "$@"
	@$(CXX) -MM *.cpp | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' >> $@

include $(BUILD)/depend.mk

