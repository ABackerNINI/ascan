# BUILD DETAILS

CC = gcc
CXX = g++
CFLAGS = -W -Wall
CXXFLAGS = -W -Wall
LFLAGS = -lm
BD = ./build

# BUILD EXECUTABLE

bin1 = main_cpp
bin2 = main_c

all: $(BD)/release $(bin1) $(bin2)

debug: CFLAGS += -g -DDEBUG=1
debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BD)/debug $(bin1) $(bin2)

rebuild: clean all

# EXECUTABLE 1

obj1 = main_cpp.o
obj1bd = $(obj1:%=$(BD)/%)

$(bin1): $(obj1bd)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# EXECUTABLE 2

obj2 = main_c.o
obj2bd = $(obj2:%=$(BD)/%)

$(bin2): $(obj2bd)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

# CLEAN UP

clean:
	rm -rf "$(bin1)" "$(bin2)" "$(BD)"

# DEPENDENCIES

main_cpp.o: main_cpp.cpp
main_c.o: main_c.c

# COMPILE TO OBJECTS

$(BD)/%.o: %.c Makefile
	@mkdir -p $(BD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BD)/%.o: %.cpp Makefile
	@mkdir -p $(BD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BD)/release:
	@if [ -e $(BD)/debug ]; then rm -rf $(BD); fi
	@mkdir -p $(BD)
	@touch $(BD)/release

$(BD)/debug:
	@if [ -e $(BD)/release ]; then rm -rf $(BD); fi
	@mkdir -p $(BD)
	@touch $(BD)/debug

# PHONY

.PHONY: all rebuild clean debug
