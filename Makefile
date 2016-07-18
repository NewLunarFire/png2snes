CC=gcc
CFLAGS=-Werror -pedantic -g
LDFLAGS=-lpng -lz
HEADERS=pngfunctions.h
SRC=main.c pngfunctions.c argparser.c
TARGET=png2snes

all: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)
