# BUILD DETAILS

CXX      = g++
CXXFLAGS = -W -Wall
LFLAGS   = -lm
BUILD    = build

# TARGETS

TARGET   = ascan

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode $(TARGET)

release: CXXFLAGS += -O3 # -DNDEBUG=1
release: $(BUILD)/release.mode $(TARGET)

# EXECUTABLE DETAILS

OBJ      = ascan.o options.o parser.o cfile.o align.o mfile.o common.o config.o

OBJBD    = $(OBJ:%=$(BUILD)/%)

$(TARGET): $(OBJBD)
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
	rm -rf "$(TARGET)" "$(BUILD)"

# PHONY

.PHONY: default debug release clean

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@rm -f "$@"
	@$(CXX) $(CXXFLAGS) -MM *.cpp | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' >> $@

include $(BUILD)/depend.mk

