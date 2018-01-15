LIBS_OPENCV = `$(PREFIX)pkg-config --libs opencv`
INCLUDE_OPENCV = `$(PREFIX)pkg-config --cflags opencv`

default: 
	gcc -std=c99 -O3 -Wall -pedantic -Wno-unused-function -Wno-unused-parameter -Wno-deprecated -Wno-deprecated-declarations -Wno-sign-compare -Wno-unused-but-set-parameter -c vibe-background-sequential.c
	g++ -o main-opencv -O3 -Wall -pedantic $(INCLUDE_OPENCV) main-opencv_NADI.cpp vibe-background-sequential.o $(LIBS_OPENCV)

