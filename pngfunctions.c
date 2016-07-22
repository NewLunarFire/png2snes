#include <stdint.h>
#include <stdlib.h>
#include <png.h>

#include "pngfunctions.h"

#define HEADER_BYTES 8

int detect_png(FILE* fp)
{
  unsigned char header[HEADER_BYTES];
  int is_png;

  //Read bytes at the beginning and verify if this is a PNG file
  fread(header, 1, HEADER_BYTES, fp);
  is_png = !png_sig_cmp(header, 0, HEADER_BYTES);

  if (!is_png)
    fprintf(stderr, "File is not a PNG");

  return is_png;
}

int initialize_libpng(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info)
{
  //Create the PNG Read Struct
  *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
  if (!png_ptr)
  {
    fprintf(stderr, "Error creating libpng read struct\n");
    return 0;
  }

  //Get the first info struct
  *info_ptr = png_create_info_struct(*png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fprintf(stderr, "Error creating first info struct\n");
    return 0;
  }

  //Get the end info struct
  *end_info = png_create_info_struct(*png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
    fprintf(stderr, "Error creating second info struct\n");
    return 0;
  }

  //Initialize IO and tell libpng we used bytes to check the header
  png_init_io(*png_ptr, fp);
  png_set_sig_bytes(*png_ptr, HEADER_BYTES);

  //Read file information
  png_read_info(*png_ptr, *info_ptr);
  return 1;
}

int detect_palette(png_structp png_ptr, png_infop info_ptr)
{
  //Verify that this is a paletted PNG image
  if(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_PALETTE)
  {
    fprintf(stderr, "File does not contain a palette\n");
    return 0;
  }

  //Verify the validity of the PLTE chunk
  if(!png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))
  {
    fprintf(stderr, "PLTE chunk is invalid\n");
    return 0;
  }

  return 1;
}

uint8_t* read_png(png_structp png_ptr, png_infop info_ptr)
{
  //Get number of rows
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //Get number of bytes per row
  size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  //Allocate memory
  png_bytep data = (png_bytep)malloc(sizeof(png_byte) * rowbytes * height);
  png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int i = 0; i < height; i++)
    row_pointers[i] =  &(data[i*rowbytes]);

  uint8_t* pixels = (uint8_t*)malloc(width * height * sizeof(uint8_t));

  //Read image
  png_read_image(png_ptr, row_pointers);

  uint and_mask = 0xFF >> (8 - bit_depth);
  uint z;

  //printf("\nbd= %u, am=%02X, m = %u\n", bit_depth, and_mask, mult);

  for(size_t y = 0; y < height; y++)
  {
    z = 0;
    for(size_t x = 0; x < width; x++, z += bit_depth)
    {
      uint right_shift = 8 - bit_depth - (z & 0x07);
      pixels[(y*width)+x] = (row_pointers[y][z >> 3] >> right_shift) & and_mask;
    }
  }

  return pixels;
}
