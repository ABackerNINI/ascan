# Build details

CXX = g++
CXXFLAGS = -W -Wall -g
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

obj = ascan.o mfile.o common.o parser.o cfile.o config.o
obj_bd = $(obj:%=$(BD)/%)

$(bin): prepare $(obj_bd)
	$(CXX) -o $(bin) $(obj_bd) $(CXXFLAGS)

# Dependencies

$(BD)/mfile.o: mfile.h cfile.h config.h flags.h common.h
$(BD)/common.o: common.h
$(BD)/parser.o: parser.h
$(BD)/cfile.o: common.h cfile.h parser.h
$(BD)/config.o: config.h parser.h
$(BD)/ascan.o: ascan.h cfile.h config.h flags.h mfile.h common.h

# Clean up

.PHONY: clean
clean:
	rm -f "$(bin)" $(obj_bd)
	rm -fd "$(BD)"
