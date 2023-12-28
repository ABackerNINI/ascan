ascan.o: ascan.cpp ascan.h cfile.h config.h options.h common.h debug.h \
 mfile.h
cfile.o: cfile.cpp cfile.h common.h debug.h parser.h
common.o: common.cpp common.h
config.o: config.cpp config.h debug.h parser.h
mfile.o: mfile.cpp mfile.h cfile.h config.h debug.h common.h options.h
opt_parser.o: opt_parser.cpp opt_parser.h debug.h
options.o: options.cpp options.h config.h common.h debug.h
parser.o: parser.cpp parser.h debug.h
