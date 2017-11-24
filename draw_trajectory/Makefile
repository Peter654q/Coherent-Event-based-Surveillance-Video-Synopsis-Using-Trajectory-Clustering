LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	g++ -o Draw_trajectory -O3 -Wall -pedantic $(INCLUDE_OPENCV) Draw_trajectory.cpp $(LIBS_OPENCV)

