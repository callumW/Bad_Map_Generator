CC=g++
LINKER=ld
FLAGS=-g -Wall -std=c++14
LINKS= -LF:\libs\SDL2-2.0.4\i686-w64-mingw32\lib -lmingw32 -lSDL2main \
	-lSDL2 -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 \
	-luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion \
	-luuid -static
SDL_FLAGS= -IF:\libs\SDL2-2.0.4\i686-w64-mingw32\include -Dmain=SDL_main


SOURCE=$(wildcard *.cpp)
OBS=$(SOURCE:.cpp=.o)

EXECUTABLE=generate.exe

all: $(OBS)
	$(CC) $(FLAGS) $(OBS) -o $(EXECUTABLE) $(LINKS)

.cpp.o:
	$(CC) $(SDL_FLAGS) $(FLAGS)  -c $< -o $@

clean:
	rm -f *.o
