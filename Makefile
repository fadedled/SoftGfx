
CC=gcc
CFLAGS= -Wall -O2 -lm -std=c99
SDL2= `sdl2-config --cflags --libs`
INCLUDE= -Iinclude/
APP_NAME= sample

all:	src/*.c src/SoftGfx/*.c
	$(CC) src/*.c src/SoftGfx/*.c -o $(APP_NAME) $(CFLAGS) $(SDL2) $(INCLUDE)

clean:
	rm -f $(APP_NAME)

