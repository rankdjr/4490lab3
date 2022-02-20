all: lab3

lab3: lab3.cpp matrix.cpp log.cpp
	g++ lab3.cpp matrix.cpp log.cpp libggfonts.a -Wall -o lab3 \
	-lX11 -lGL -lGLU -lm -lrt

clean:
	rm -f lab3 *.o

