CC = gcc
CFLAGS = -g -std=c11 -Wall -Wextra -pedantic

CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Wextra -pedantic

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

C_SRCS = pnglite.c
CXX_SRCS = main.cpp plot.cpp bounds.cpp image.cpp exception.cpp \
	reader.cpp renderer.cpp expr.cpp expr_parser.cpp func.cpp \
	fill.cpp

TEST_PLOT_SRCS = test_plot.cpp bounds.cpp fill.cpp image.cpp \
	plot.cpp func.cpp exception.cpp

OBJS = $(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
TEST_PLOT_OBJS = $(C_SRCS:.c=.o) $(TEST_PLOT_SRCS:.cpp=.o)

plot : $(OBJS)
	$(CXX) -o $@ $(OBJS) -lm -lz

test_plot : $(TEST_PLOT_OBJS)
	$(CXX) -o $@ $(TEST_PLOT_OBJS) -lm -lz

clean :
	rm -f *.o plot test_plot depend.mak

depend :
	$(CC) -M $(CFLAGS) $(C_SRCS) > depend.mak
	$(CXX) -M $(CXXFLAGS) $(CXX_SRCS) >> depend.mak

depend.mak :
	touch $@

include depend.mak
