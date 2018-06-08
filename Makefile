
all:
	g++ -o chip8.out -O2 main.cpp chip8.cpp -std=c++11 -D_REENTRANT -I/usr/include/SDL2 -lSDL2

clean:
	rm chip8.out
