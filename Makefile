# BUILD DETAILS

CXX = g++
CXXFLAGS = -W -Wall -g -DDEBUG=1 -DDISABLE_WRITE=0
LFLAGS =
BD = ./build

# BUILD EXECUTABLE

bin = ascan

all: $(bin)

obj = ascan.o options.o parser.o cfile.o mfile.o common.o config.o align.o

# CLEAN UP

clean:
	rm -rf "$(bin)" "$(BD)"

# EXECUTABLE 1

objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# COMPILE TO OBJECTS

$(BD)/%.o: %.cpp
	@mkdir -p "$(BD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BD)/depend.mk: $(SRC)
	@mkdir -p "$(BD)"
	@rm -f "$(BD)/depend.mk"
	@$(CXX) -MM *.cpp | sed 's/^\(.*\).o:/$$(BD)\/\1.o:/' >> $@

include $(BD)/depend.mk

# PHONY

.PHONY: all depend clean
