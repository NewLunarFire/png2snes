#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <png.h>

#include "pngfunctions.h"
#include "argparser.h"

#define DEFAULT_TILE_SIZE 8
#define CONVERT_TO_SNES_COLOR(red, green, blue) (((blue & 0xF8) << 7) | ((green & 0xF8) << 2) | (red >> 3))

void convert_palette(png_structp png_ptr, png_infop info_ptr, FILE* output);
void convert_to_tiles(png_structp png_ptr, png_infop info_ptr, FILE* output, int bitplane_count, int tilesize);
void convert_tile(png_bytep* row_pointers, int column, int bitplane_count, int bit_depth, FILE* output);
void output_byte_text(uint8_t byte, FILE* output);

uint8_t* convert_to_bitplanes(png_bytep row_pointer, int col, int bitplane_count, int bit_depth);

int binary, verbose;

//Lib PNG Structures
png_structp png_ptr;
png_infop info_ptr, end_info;

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

FILE* open_file(const char* basename, const char* extension, const char* mode)
{
  char* filename = (char*)malloc((strlen(basename) + 5) * sizeof(char));
  strcpy(filename, basename);
  strcat(filename, ".");
  strcat(filename, extension);
  return fopen(filename, mode);
}

int main(int argc, char *argv[])
{
  FILE* input = NULL; //File containing png image
  FILE* output = NULL; //Output file

  //Parse command-line arguments
  struct arguments args = parse_arguments(argc, argv);
  int bitplane_count, tilesize;

  verbose = args.verbose;
  binary = args.binary;

  //Open the file
  input = fopen(args.input_file, "rb");
  if (!input)
  {
    fprintf(stderr, "Error opening file %s\n", args.input_file);
    return -1;
  }

  //If file is not a PNG, quit
  if (!detect_png(input))
    return close_file_exit(input, -2);

  //If initializing libpng failed, quit
  if(!initialize_libpng(input, &png_ptr, &info_ptr, &end_info))
    return close_file_exit(input, -3);

  if(!detect_palette(png_ptr, info_ptr))
    return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, -4);

  //Print tilesize
  if(args.tilesize == 0)
  {
    tilesize = DEFAULT_TILE_SIZE;
    if(verbose)
      fprintf(stderr, "Assuming %dx%d background tiles\n", tilesize, tilesize);
  }
  else
  {
    tilesize = args.tilesize;
    if(verbose)
      fprintf(stderr, "Using %dx%d background tiles\n", tilesize, tilesize);
  }

  //Print bitplanes used
  if(args.bitplanes != 0)
  {
    bitplane_count = args.bitplanes;
    if(verbose)
      fprintf(stderr, "Using %d bitplanes\n", bitplane_count);
  }
  else
  {
    //Get bit depth from png file
    int bitplane_count = png_get_bit_depth(png_ptr, info_ptr);
    if(bitplane_count < 2)
      bitplane_count = 2;

    if(verbose)
      fprintf(stderr, "Assuming %d bitplanes\n", bitplane_count);
  }

  //Open output file for CGRAM
  if(strcmp(args.output_file, "-") == 0)
  {
    output = stdout;
  }
  else
  {
    char* ext = (binary ? "cgr" : "asm");
    output = open_file(args.output_file, ext, "w");

    if(!output)
    {
      fprintf(stderr, "Could not create file %s.%s, aborting...\n", args.output_file, ext);
      return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, -5);
    }
  }

  //Convert palette to the format used by the SNES
  convert_palette(png_ptr, info_ptr, output);

  //Open output file for VRAM
  if(binary)
  {
    fclose(output);
    output = open_file(args.output_file, "vra", "w");

    if(!output)
    {
      fprintf(stderr, "Could not create file %s.vra, aborting...\n", args.output_file);
      return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, -5);
    }
  }

  //Convert image data in PNG to tiles
  convert_to_tiles(png_ptr, info_ptr, output, bitplane_count, tilesize);

  fclose(output);
  return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, 0);
}

void convert_palette(png_structp png_ptr, png_infop info_ptr, FILE* output)
{
  png_color* palette;
  int num_palette;

  //Fetch Palette from PNG file
  png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

  if(verbose)
  {
    fprintf(stderr, "Palette contains %d colors\n", num_palette);
    fprintf(stderr, "CGRAM section is %d bytes long\n", num_palette * 2);
  }

  if(!binary)
  {
    //Start outputting data in WLA-DX format
    fprintf(output, "PALETTE:\n");
    fprintf(output, "\t.dw ");
  }

  for(unsigned int i = 0; i < num_palette; i++)
  {
    uint16_t nes_color = CONVERT_TO_SNES_COLOR(palette[i].red, palette[i].green, palette[i].blue);

    if(binary)
    {
      putc(nes_color >> 8, output);
      putc(nes_color & 0xFF, output);
    }
    else
    {
      fprintf(output, "$%04X", nes_color);

      if(i < (num_palette - 1))
        fprintf(output, ", ");
    }
  }

  if(!binary)
    fprintf(output, "\n");
}

void convert_to_tiles(png_structp png_ptr, png_infop info_ptr, FILE* output, int bitplane_count, int tilesize)
{
  //Get image size and bit depth
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //Get number of horizontal and vertical tiles to process
  unsigned int horizontal_tiles = width / DEFAULT_TILE_SIZE;
  unsigned int vertical_tiles = height / DEFAULT_TILE_SIZE;

  //Keep tracks of outputed bytes
  unsigned int output_bytes = 0;

  //Get number of bytes per row
  size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  //Allocate space for 8 rows
  png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * DEFAULT_TILE_SIZE);
  for(int i = 0; i < DEFAULT_TILE_SIZE; i++)
    row_pointers[i] =  (png_bytep)malloc(sizeof(png_byte) * rowbytes);

  //Print information on the image
  fprintf(stderr, "Image size is %dx%d, bit depth is %d\n", width, height, bit_depth);
  fprintf(stderr, "Tile count: %d\n", horizontal_tiles * vertical_tiles);
  fprintf(stderr, "VRAM section is %d bytes long\n", bitplane_count * DEFAULT_TILE_SIZE * horizontal_tiles * vertical_tiles);

  if(!binary)
    fprintf(output, "TILES:");

  for(size_t i = 0; i < vertical_tiles; i++)
  {
    //Read rows
    png_read_rows(png_ptr, row_pointers, NULL, DEFAULT_TILE_SIZE);

    //For each horizontal tile
    for(size_t j = 0; j < horizontal_tiles; j++)
      convert_tile(row_pointers, j, bitplane_count, bit_depth, output);
  }
}

void convert_tile(png_bytep* row_pointers, int column, int bitplane_count, int bit_depth, FILE* output)
{
  uint8_t** rows = (uint8_t**)malloc(DEFAULT_TILE_SIZE * sizeof(uint8_t*));

  //For each row in the tile
  for(size_t i = 0; i < DEFAULT_TILE_SIZE; i++)
    rows[i] = convert_to_bitplanes(row_pointers[i], column, bitplane_count, bit_depth);

  for(size_t i = 0; i < bitplane_count; i+=2)
  {
    for(size_t j = 0; j < DEFAULT_TILE_SIZE; j++)
    {
        if(!binary)
        {
          //Output two bitplanes for each
          output_byte_text(rows[j][i], output);
          output_byte_text(rows[j][i+1], output);
        }
        else
        {
          putc(rows[j][i], output);
          putc(rows[j][i+1], output);
        }
    }
  }
}

uint8_t* convert_to_bitplanes(png_bytep row_pointer, int col, int bitplane_count, int bit_depth)
{
  uint8_t* bitplanes = (uint8_t*)malloc(sizeof(uint8_t) * bitplane_count);
  uint and_mask = 0xFF >> (8 - bit_depth);

  for(size_t i = 0; i < bitplane_count; i++)
    bitplanes[i] = 0;

  for(size_t i = 0; i < DEFAULT_TILE_SIZE; i++)
  {
    //Calculate number of bits to shift right
    uint row = DEFAULT_TILE_SIZE - i - 1;
    uint right_shift = bit_depth * row;

    uint offset = right_shift / 8;
    right_shift %= 8;

    //Expand to 8-bits palette index
    uint8_t color = (row_pointer[(col*bit_depth)+offset] >> right_shift) & and_mask;

    //Store on bitplanes
    for(size_t j = 0; j < bitplane_count; j++)
    {
      uint plane = bitplane_count-j-1;
      bitplanes[j] |= (((color & (1 << plane))) >> plane) << row;
    }
  }

  return bitplanes;
}

void output_byte_text(uint8_t byte, FILE* output)
{
  static int output_bytes = 0;

  if((output_bytes % 16) == 0)
    fprintf(output, "\n\t.db ");

  fprintf(output, "$%02X", byte);

  if((output_bytes % 16) < 15)
    fprintf(output, ",  ");

  output_bytes++;
}
