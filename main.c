#include <png.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "argparser.h"
#include "palette.h"
#include "pngfunctions.h"
#include "tile.h"

#define DEFAULT_TILE_SIZE 8

void generate_cgram(png_structp png_ptr, png_infop info_ptr, struct arguments args);
void generate_vram(png_structp png_ptr, png_infop info_ptr, struct arguments args);
uint8_t* convert_to_tiles(uint8_t* data, uint width, uint height, uint bitplane_count, uint tilesize, uint* data_size);

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
  //Lib PNG Structures
  png_structp png_ptr;
  png_infop info_ptr, end_info;

  FILE* input = NULL; //File containing png image
  FILE* output = NULL; //Output file

  //Parse command-line arguments
  struct arguments args = parse_arguments(argc, argv);

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

  //If the PNG file does not contain a palette, exit
  if(!detect_palette(png_ptr, info_ptr))
    return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, -4);

  generate_cgram(png_ptr, info_ptr, args);
  generate_vram(png_ptr, info_ptr, args);

  return close_file_and_png_exit(input, &png_ptr, &info_ptr, &end_info, 0);
}

void generate_cgram(png_structp png_ptr, png_infop info_ptr, struct arguments args)
{
  //Convert palette to the format used by the SNES
  int palette_size;
  uint16_t* palette = convert_palette(png_ptr, info_ptr, &palette_size);

  if(args.verbose)
  {
    fprintf(stderr, "Palette contains %d colors\n", palette_size);
    fprintf(stderr, "CGRAM section is %d bytes long\n", palette_size * 2);
  }

  if(args.binary)
    output_palette_binary(args.output_file, palette, palette_size);
  else
    output_palette_wla(args.output_file, palette, palette_size);

    free(palette);
}

void generate_vram(png_structp png_ptr, png_infop info_ptr, struct arguments args)
{
  unsigned int bitplane_count, tilesize, data_size;

  //Get image size and bit depth
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //Print tilesize
  if(args.tilesize == 0)
  {
    tilesize = DEFAULT_TILE_SIZE;
    if(args.verbose)
      fprintf(stderr, "Assuming %ux%u background tiles\n", tilesize, tilesize);
  }
  else
  {
    tilesize = args.tilesize;
    if(args.verbose)
      fprintf(stderr, "Using %ux%u background tiles\n", tilesize, tilesize);
  }

  //Print bitplanes used
  if(args.bitplanes != 0)
  {
    bitplane_count = args.bitplanes;
    if(args.verbose)
      fprintf(stderr, "Using %u bitplanes\n", bitplane_count);
  }
  else
  {
    //Get bit depth from png file
    bitplane_count = bit_depth;
    if(bitplane_count < 2)
      bitplane_count = 2;

    if(args.verbose)
      fprintf(stderr, "Assuming %u bitplanes\n", bitplane_count);
  }

  //Get number of horizontal and vertical tiles to process
  unsigned int horizontal_tiles = width / tilesize;
  unsigned int vertical_tiles = height / tilesize;

  //Print information on the image
  if(args.verbose)
  {
    fprintf(stderr, "Image size is %ux%u, bit depth is %u\n", width, height, bit_depth);
    fprintf(stderr, "Tile count: %u (%ux%u)\n", horizontal_tiles * vertical_tiles, horizontal_tiles, vertical_tiles);
    fprintf(stderr, "Outputting on %u bitplanes\n", bitplane_count);
  }

  uint8_t* pixels = read_png(png_ptr, info_ptr);
  uint8_t* data;

  if(tilesize == 8)
    data = convert_to_tiles_8_8(pixels, width, height, bitplane_count, &data_size);
  else if(tilesize == 16)
    data = convert_to_tiles_16_16(pixels, width, height, bitplane_count, &data_size);

  if(args.verbose)
    fprintf(stderr, "VRAM section is %u bytes long\n", data_size);

  if(args.binary)
    output_tiles_binary(args.output_file, data, data_size);
  else
    output_tiles_wla(args.output_file, data, data_size);

  free(pixels);
  free(data);
}
