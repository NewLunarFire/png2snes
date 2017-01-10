#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "argparser.h"
//#include "main.h"
//#include "palette.h"
#include "pngfunctions.h"
#include "tile.h"

uint8_t* get_tile_from_png(uint8_t* destination, png_structp png_ptr, png_bytepp row_pointers, int x, int y);
void convert_to_bitplanes(uint8_t* destination, const uint8_t* source, int bitplane_count);

//Tests
int testGetTileFromPNG();
int testConvert0sto2Bitplanes();
int testConvert3sto2Bitplanes();
int testConvertTo2Bitplanes();


struct unit_test_t {
  char* name;
  int (*testFunction)();
};

struct unit_test_t unitTests[] = {
  {"Get Tile from PNG", testGetTileFromPNG},
  {"Convert 0s to 2 bitplanes", testConvert0sto2Bitplanes},
  {"Convert 3s to 2 bitplanes", testConvert3sto2Bitplanes},
  {"Convert Array to 2 bitplanes", testConvertTo2Bitplanes},
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
  png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);

  for(i = 0; i < height; i++)
    row_pointers[i] = malloc(sizeof(png_byte) * width);

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
          printf("Expected %u got %u\n", v, tile[k]);
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

int testConvert0sto2Bitplanes() {
  uint8_t data[64];
  uint8_t bitplanes[16];
  size_t i;
  int exit_code = 0;

  memset(data, 0, 64);

  convert_to_bitplanes(bitplanes, data, 2);

  for(i = 0; i < 16; i++) {
    if(bitplanes[i] != 0) {
      printf("Expected all zeroes, but got a %02X at %lu\n", bitplanes[i], i);
      exit_code = 1;
    }
  }

  return exit_code;
}

int testConvert3sto2Bitplanes() {
  uint8_t data[64];
  uint8_t bitplanes[16];
  size_t i;
  int exit_code = 0;

  memset(data, 3, 64);

  convert_to_bitplanes(bitplanes, data, 2);

  for(i = 0; i < 16; i++) {
    if(bitplanes[i] != 0xFF) {
      printf("Expected all FFs, but got a %02X at %lu\n", bitplanes[i], i);
      exit_code = 1;
    }
  }

  return exit_code;
}

int testConvertTo2Bitplanes() {
  uint8_t actualResult[16];
  size_t i;

  const uint8_t inputSequence[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0,
                                 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1,
                                 2, 2, 2, 2, 2, 2, 2, 2,
                                 2, 2, 2, 2, 2, 2, 2, 2,
                                 3, 3, 3, 3, 3, 3, 3, 3,
                                 3, 3, 3, 3, 3, 3, 3, 3};

  const uint8_t expectedResult[] = {0x00, 0x00, 0x00, 0x00,
                                    0xFF, 0x00, 0xFF, 0x00,
                                    0x00, 0xFF, 0x00, 0xFF,
                                    0xFF, 0xFF, 0xFF, 0xFF};


  convert_to_bitplanes(actualResult, inputSequence, 2);
  if(memcmp(actualResult, expectedResult, 16) != 0) {
    printf("Expected and actual results do not match\n");

    printf("Expected:");
    for(i = 0; i < 15; i++)
      printf("%02X, ", expectedResult[i]);
    printf("%02X\n", expectedResult[15]);

    printf("Actual:");
    for(i = 0; i < 15; i++)
      printf("%02X, ", actualResult[i]);
    printf("%02X\n", actualResult[15]);

    return 1;
  }

  return 0;
}
