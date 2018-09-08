CC=gcc
CFLAGS=-std=c99 -Wall -pedantic -g -D_GNU_SOURCE `libpng-config --cflags`
LDFLAGS=`libpng-config --ldflags`
HEADERS=argparser.h palette.h pngfunctions.h tile.h
SRC=argparser.c palette.c pngfunctions.c tile.c
TARGET=png2snes

all: main.c main.h $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) main.c $(LDFLAGS) -o $(TARGET)

test: tests.c $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) tests.c $(LDFLAGS) -o $@

clean:
	rm -f *.asm *.cgr *.vra
