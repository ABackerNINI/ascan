# BUILD DETAILS

CXX = g++
CXXFLAGS = -W -Wall -g -DDEBUG=1 -DDISABLE_WRITE=0
LFLAGS =
BD = ./build

# BUILD EXECUTABLE

bin = ascan

all: $(bin)

rebuild: clean all

# EXECUTABLE 1

obj = ascan.o options.o parser.o cfile.o mfile.o common.o config.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# CLEAN UP

clean:
	rm -rf "$(bin)" "$(BD)"

# DEPENDENCIES

$(BD)/options.o: options.h config.h common.h debug.h
$(BD)/parser.o: parser.h debug.h
$(BD)/ascan.o: ascan.h cfile.h config.h options.h common.h debug.h mfile.h
$(BD)/cfile.o: cfile.h common.h debug.h parser.h
$(BD)/mfile.o: mfile.cpp mfile.h cfile.h config.h debug.h common.h options.h
$(BD)/common.o: common.h
$(BD)/config.o: config.h parser.h

# COMPILE TO OBJECTS

$(BD)/%.o: %.cpp
	@mkdir -p "$(BD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# PHONY

.PHONY: all rebuild clean
