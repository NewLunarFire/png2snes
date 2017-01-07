CC=gcc
CFLAGS=-std=c99 -Werror -pedantic -g
LDFLAGS=-lpng -lz
HEADERS=pngfunctions.h
SRC=main.c pngfunctions.c argparser.c palette.c tile.c
TARGET=png2snes

all: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

test: tests.c
	$(CC) $< -o $@

clean:
	rm *.asm *.cgr *.vra
