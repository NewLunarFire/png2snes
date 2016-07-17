CC=gcc
CFLAGS=-Werror -pedantic -g
LDFLAGS=-lpng -lz
SRC=main.c
TARGET=png2snes

all: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)
