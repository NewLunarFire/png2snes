#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//#include "argparser.h"
//#include "main.h"
//#include "palette.h"
#include "pngfunctions.h"
#include "tile.h"

uint8_t* get_tile_from_png(uint8_t* destination, png_structp png_ptr, png_bytepp row_pointers, int x, int y);

//Tests
int testGetTileFromPNG();

struct unit_test_t {
  char* name;
  int (*testFunction)();
};

struct unit_test_t unitTests[] = {
  {"get_tile_from_png", testGetTileFromPNG},
  {NULL, NULL}
};

int main(int argc, char* argv[]) {
  unsigned int i = 0;
  unsigned int testsPassed = 0;
  unsigned int testsFailed = 0;

  printf("PNG2SNES test suite\n\n");
  for(i = 0; unitTests[i].name != NULL; i++) {
    printf("=====================================\n");
    printf("Test %d: %s\n", i+1, unitTests[i].name);
    printf("=====================================\n");
    int result = unitTests[i].testFunction();

    printf("=====================================\n");
    if(result != 0) {
      printf("Failed, returned %d exit status\n", result);
      testsFailed++;
    } else {
      printf("Success\n");
      testsPassed++;
    }
    printf("=====================================\n");
  }

  printf("Stats: %d run, %d passed, %d failed\n", i, testsPassed, testsFailed);
  return testsFailed > 1;
}

png_bytepp get_row_pointers(png_structp png_ptr, unsigned int width, unsigned int height) {
  size_t i;
  png_bytepp row_pointers = (png_bytepp)malloc(sizeof(png_bytep) * height);

  for(i = 0; i < height; i++)
    row_pointers[i] = (png_bytep)malloc(sizeof(png_byte) * width);

  png_read_image(png_ptr, row_pointers);
  return row_pointers;
}

void free_row_pointers(png_bytepp row_pointers, unsigned int height) {
  size_t i;
  for(i = 0; i < height; i++)
    free(row_pointers[i]);

  free(row_pointers);
}

int testGetTileFromPNG() {
  int exit_code = 0;
  uint8_t tile[TILE_SIZE];
  size_t col, row, k;

  //Lib PNG Structures
  png_structp png_ptr;
  png_infop info_ptr, end_info;
  png_bytepp row_pointers;

  //Open the file
  FILE* input = fopen("ChessPattern.png", "rb"); //File containing png image

  if (!input)
  {
    //Could not open file
    printf("Error opening file ChessPattern.png\n");
    return -1;
  }

  //If file is not a PNG, quit
  if (!detect_png(input)) {
    fclose(input);
    return -1;
  }

  //If initializing libpng failed, quit
  if(!initialize_libpng(input, &png_ptr, &info_ptr, &end_info)) {
    fclose(input);
    return -1;
  }

  row_pointers = get_row_pointers(png_ptr, 16, 16);

  for(row = 0; row < 2; row++) {
    for(col = 0; col < 2; col++) {
      uint8_t v = (row * 2) + col;
      get_tile_from_png(tile, png_ptr, row_pointers, col, row);

      for(k = 0; k < TILE_SIZE; k++) {
        if(tile[k] != v) {
          printf("Expected %lu got %lu\n", v, tile[k]);
          row = col = 2;
          k = TILE_SIZE;
          exit_code = 1;
        }
      }
    }
  }

  free_row_pointers(row_pointers, 16);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(input);
  return exit_code;
}
