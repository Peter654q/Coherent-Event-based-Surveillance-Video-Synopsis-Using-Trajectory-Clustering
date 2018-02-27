LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o object_tracking -O3 -Wall -pedantic $(INCLUDE_OPENCV) object_tracking.cpp $(LIBS_OPENCV)

