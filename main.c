#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <png.h>

#include "pngfunctions.h"
#include "argparser.h"

#define DEFAULT_TILE_SIZE 8
#define CONVERT_TO_SNES_COLOR(red, green, blue) (((blue & 0xF8) << 7) | ((green & 0xF8) << 2) | (red >> 3))

void convert_palette(png_structp png_ptr, png_infop info_ptr);
void convert_to_tiles(png_structp png_ptr, png_infop info_ptr);
uint16_t convert_to_bitplanes(png_bytep row_pointer, int col_offset);

int parse (int argc, char **argv);

uint bitplanes;
uint tilesize;

static inline int close_file_exit(FILE* fp, int exit_code)
{
  fclose(fp);
  exit(exit_code);
}

static inline int close_file_and_png_exit(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info, int exit_code)
{
  png_destroy_read_struct(png_ptr, info_ptr, end_info);
  close_file_exit(fp, exit_code);
}

int main(int argc, char *argv[])
{
  //File containing png image
  FILE *fp;

  //Lib PNG Structures
  png_structp png_ptr;
  png_infop info_ptr, end_info;

  //Parse command-line arguments
  struct arguments args = parse_arguments(argc, argv);

  //Open the file
  fp = fopen(args.input_file, "rb");
  if (!fp)
  {
    printf("Error opening file %s\n", args.input_file);
    return -1;
  }

  //If file is not a PNG, quit
  if (!detect_png(fp))
    return close_file_exit(fp, -2);

  //If initializing libpng failed, quit
  if(!initialize_libpng(fp, &png_ptr, &info_ptr, &end_info))
    return close_file_exit(fp, -3);

  if(!detect_palette(png_ptr, info_ptr))
    return close_file_and_png_exit(fp, &png_ptr, &info_ptr, &end_info, -4);

  //Print tilesize
  if(args.tilesize == 0)
    fprintf(stderr, "Assuming %dx%d background tiles\n", DEFAULT_TILE_SIZE, DEFAULT_TILE_SIZE);
  else
    fprintf(stderr, "Using %dx%d background tiles\n", args.tilesize, args.tilesize);

  //Print bitplanes used
  if(args.bitplanes != 0)
    fprintf(stderr, "Using %d bitplanes\n", args.bitplanes);
  else
  {
    //Get bit depth from png file
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    if(bit_depth < 2)
      bit_depth = 2;
    fprintf(stderr, "Assuming %d bitplanes\n", bit_depth);
  }

  //Convert palette to the format used by the SNES
  convert_palette(png_ptr, info_ptr);

  //Convert image data in PNG to tiles
  convert_to_tiles(png_ptr, info_ptr);

  return close_file_and_png_exit(fp, &png_ptr, &info_ptr, &end_info, 0);
}

void convert_palette(png_structp png_ptr, png_infop info_ptr)
{
  png_color* palette;
  int num_palette;

  //Fetch Palette from PNG file
  png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

  fprintf(stderr, "Palette contains %d colors\n", num_palette);
  fprintf(stderr, "CGRAM section is %d bytes long\n", num_palette * 2);

  //Start outputting data in WLA-DX format
  fprintf(stdout, "PALETTE:\n");
  fprintf(stdout, "\t.dw ");

  for(unsigned int i = 0; i < num_palette; i++)
  {
    uint16_t nes_color = CONVERT_TO_SNES_COLOR(palette[i].red, palette[i].green, palette[i].blue);
    fprintf(stdout, "$%04X", nes_color);

    if(i < (num_palette - 1))
      fprintf(stdout, ", ");
  }
  fprintf(stdout, "\n");

}

void convert_to_tiles(png_structp png_ptr, png_infop info_ptr)
{
  //Get image size and bit depth
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //Get number of horizontal and vertical tiles to process
  unsigned int horizontal_tiles = width / DEFAULT_TILE_SIZE;
  unsigned int vertical_tiles = height / DEFAULT_TILE_SIZE;

  //Get number of bytes per row
  size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  //Allocate space for 8 rows
  png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * DEFAULT_TILE_SIZE);
  for(int i = 0; i < DEFAULT_TILE_SIZE; i++)
    row_pointers[i] =  (png_bytep)malloc(sizeof(png_byte) * rowbytes);

  //Print information on the image
  fprintf(stderr, "Image size is %dx%d, bit depth is %d\n", width, height, bit_depth);
  fprintf(stderr, "Tile count: %d\n", horizontal_tiles * vertical_tiles);
  fprintf(stderr, "VRAM section is %d bytes long\n", 2 * DEFAULT_TILE_SIZE * horizontal_tiles * vertical_tiles);

  fprintf(stdout, "TILES:\n");
  for(size_t i = 0; i < vertical_tiles; i++)
  {
    //Read rows
    png_read_rows(png_ptr, row_pointers, NULL, DEFAULT_TILE_SIZE);

    //For each horizontal tile
    for(size_t j = 0; j < horizontal_tiles; j++)
    {
      fprintf(stdout, "\t.db ");

      //For each row in the tile
      for(size_t k = 0; k < DEFAULT_TILE_SIZE; k++)
      {
        uint16_t data = convert_to_bitplanes(row_pointers[k], j);
        fprintf(stdout, "$%02X, $%02X", data >> 8, data & 0xFF);

        if(k < (DEFAULT_TILE_SIZE-1))
          fprintf(stdout, ", ");
      }

        fprintf(stdout, "\n");
    }
  }
}

uint16_t convert_to_bitplanes(png_bytep row_pointer, int col_offset)
{
  uint8_t bitplane1 = 0, bitplane2 = 0;
  for(int i = 0; i < DEFAULT_TILE_SIZE; i++)
  {
    //Expand to 8-bits palette index
    char color = (row_pointer[col_offset] >> (DEFAULT_TILE_SIZE-1-i)) & 0x01;

    //Store on 2 bitplanes
    bitplane1 |= (color & 0x02) << (DEFAULT_TILE_SIZE-1-i);
    bitplane2 |= (color & 0x01) << (DEFAULT_TILE_SIZE-1-i);
  }

  return (bitplane1 << 8) | bitplane2;
}
