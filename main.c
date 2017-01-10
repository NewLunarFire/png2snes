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
uint8_t* convert_to_tiles(uint8_t* data, unsigned int width, unsigned int height, unsigned int bitplane_count, unsigned int tilesize, unsigned int* data_size);

int main(int argc, char *argv[])
{
  int exit_code = -2;

  //Lib PNG Structures
  png_structp png_ptr;
  png_infop info_ptr, end_info;

  FILE* input = NULL; //File containing png image

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
    goto close_file;

  //If initializing libpng failed, quit
  if(!initialize_libpng(input, &png_ptr, &info_ptr, &end_info))
    goto close_file;

  //If the PNG file does not contain a palette, exit
  if(!detect_palette(png_ptr, info_ptr))
    goto clean_png_struct;

  generate_cgram(png_ptr, info_ptr, args);
  generate_vram(png_ptr, info_ptr, args);

  exit_code++;
clean_png_struct:
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  exit_code++;
close_file:
  fclose(input);

  return exit_code;
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
  //unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

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

  //Print information on the image
  if(args.verbose)
  {
    fprintf(stderr, "Image size is %ux%u, bit depth is %u\n", width / tilesize, height / tilesize, bit_depth);
    fprintf(stderr, "Tile count: %u (%ux%u)\n", (width / tilesize) * (height / tilesize), width / tilesize, height / tilesize);
    fprintf(stderr, "Outputting on %u bitplanes\n", bitplane_count);
  }

  uint8_t* data = convert_tiles(png_ptr, info_ptr, bitplane_count, tilesize, &data_size);

  if(args.verbose)
    fprintf(stderr, "VRAM section is %u bytes long\n", data_size);

  if(args.binary)
    output_tiles_binary(args.output_file, data, data_size);
  else
    output_tiles_wla(args.output_file, data, data_size);

  free(data);
}
