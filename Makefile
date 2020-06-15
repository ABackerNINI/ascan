# Build details

CXX = g++
CXXFLAGS = -W -Wall -g -DDEBUG=1 -DDISABLE_WRITE=0
BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Build Executable

bin = ascan

.PHONY: all
all: $(bin)

.PHONY: prepare
prepare:
	$(if $(wildcard $(BD)),,mkdir -p $(BD))

.PHONY: rebuild
rebuild: clean all

# Executable 1

obj = ascan.o mfile.o options.o parser.o config.o common.o cfile.o
objbd = $(obj:%=$(BD)/%)

$(bin): prepare $(objbd)
	$(CXX) -o $(bin) $(objbd) $(CXXFLAGS)

# Dependencies

$(BD)/ascan.o: ascan.h cfile.h config.h options.h mfile.h common.h
$(BD)/mfile.o: mfile.h cfile.h config.h common.h options.h
$(BD)/options.o: options.h config.h common.h
$(BD)/parser.o: parser.h
$(BD)/config.o: config.h parser.h
$(BD)/common.o: common.h
$(BD)/cfile.o: common.h cfile.h parser.h

# Clean up

.PHONY: clean
clean:
	rm -f "$(bin)" $(objbd)
	rm -fd "$(BD)"
