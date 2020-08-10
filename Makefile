# Build details

CXX = g++
CXXFLAGS = -W -Wall -g -DDEBUG=1 -DDISABLE_WRITE=0
BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	-$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Build Executable

bin = ascan

all: $(bin)

rebuild: clean all

# Executable 1

obj = ascan.o options.o parser.o cfile.o mfile.o common.o config.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) -o $(bin) $(objbd) $(CXXFLAGS)

# Dependencies

$(BD)/options.o: options.h config.h common.h debug.h
$(BD)/parser.o: parser.h debug.h
$(BD)/ascan.o: ascan.h cfile.h config.h options.h mfile.h debug.h common.h
$(BD)/cfile.o: cfile.h common.h parser.h debug.h
$(BD)/mfile.o: mfile.h cfile.h config.h debug.h common.h options.h
$(BD)/common.o: common.h
$(BD)/config.o: config.h parser.h

# Clean up

clean:
	rm -f "$(bin)" $(objbd)
	rm -fd "$(BD)"

# PHONY

.PHONY: all rebuild clean
