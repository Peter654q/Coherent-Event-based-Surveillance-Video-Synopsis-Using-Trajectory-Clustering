LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o Object_tracking -O3 -Wall -pedantic $(INCLUDE_OPENCV) Object_tracking.cpp $(LIBS_OPENCV)

