# Auto generated by ascan, alpha version.
# - ver: 0.0.1
# - date: 2019/11/6
# - url: git@github.com:ABackerNINI/ascan.git

# Build details

_CXX                    = g++
_CXXFLAGS               = -W -Wall -g
_BD						= ./build

# Compile to objects

$(_BD)/%.o: %.cpp
	$(_CXX) $(_CXXFLAGS) -c -o $@ $<

# Build Executable

.PHONY: all
all: $(_BD)/ascan

.PHONY: rebuild
rebuild: clean all

# executable 1
_exe1 = $(_BD)/ascan
_objects1 = ascan.o mfile.o common.o parser.o cfile.o config.o
_objects1_b = $(_objects1:%=$(_BD)/%)

$(_BD)/ascan: $(_objects1_b)
	$(_CXX) $(_CXXFLAGS) -o $(_exe1) $(_objects1_b)

# Dependencies

$(_BD)/mfile.o: mfile.h cfile.h config.h
$(_BD)/common.o: common.h
$(_BD)/parser.o: parser.h
$(_BD)/cfile.o: common.h cfile.h parser.h
$(_BD)/config.o: config.h parser.h
$(_BD)/ascan.o: ascan.h cfile.h config.h mfile.h common.h

# Clean up

.PHONY: clean
clean:
	rm -f "$(_exe1)" $(_objects1_b)
