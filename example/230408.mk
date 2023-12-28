# BUILD DETAILS

CC 	   = gcc
CFLAGS = -W -Wall -g
LFLAGS = -lm
BUILD  = build

# BUILD EXECUTABLE

TARGET = main

all: $(TARGET)

# CLEAN UP

clean:
	rm -rf "$(TARGET)" "$(BUILD)"

# EXECUTABLE 1

OBJ    = main.o
OBJBD  = $(OBJ:%=$(BUILD)/%)

$(TARGET): $(OBJBD)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.c
	@mkdir -p "$(BUILD)"
	$(CC) $(CFLAGS) -c -o $@ $<

# PHONY

.PHONY: all clean

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@rm -f "$@"
	@$(CC) -MM *.c | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' >> $@

include $(BUILD)/depend.mk
