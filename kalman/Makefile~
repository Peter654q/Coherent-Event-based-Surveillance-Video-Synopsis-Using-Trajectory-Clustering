LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o test -O3 -Wall -Werror -pedantic $(INCLUDE_OPENCV) kalman.cpp $(LIBS_OPENCV)

