LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o video -O3 -Wall -pedantic $(INCLUDE_OPENCV) video.cpp $(LIBS_OPENCV)

