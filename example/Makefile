# BUILD DETAILS

CXX = g++
CXXFLAGS = -W -Wall -g -DDEBUG=1 -DDISABLE_WRITE=0
LFLAGS =
BD = ./build

# BUILD EXECUTABLE

bin = ascan

all: $(bin)

obj = ascan.o options.o parser.o cfile.o mfile.o common.o config.o

# EXECUTABLE 1

objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

# CLEAN UP

clean:
	rm -rf "$(bin)" "$(BD)"

# COMPILE TO OBJECTS

$(BD)/%.o: %.cpp
	@mkdir -p "$(BD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# DEPENDENCIES

depend: $(BD)/depend.mak

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BD)/depend.mak: $(SRC)
	@mkdir -p "$(BD)"
	@rm -f "$(BD)/depend.mak"
	@$(CXX) -MM *.cpp | sed 's/^\(.*\).o:/$$(BD)\/\1.o:/' >> $@

include $(BD)/depend.mak

# PHONY

.PHONY: all depend clean
