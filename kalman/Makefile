LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o kalman -O3 -Wall -pedantic $(INCLUDE_OPENCV) kalman.cpp $(LIBS_OPENCV)

