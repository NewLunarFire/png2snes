#include <stdint.h>
#include <stdlib.h>
#include <png.h>

#include "pngfunctions.h"

#define HEADER_BYTES 8

void expand_to_byte(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
  unsigned int pixels_per_byte = 8 / row_info->bit_depth;
  unsigned int mask = 0xFF >> (8 - row_info->bit_depth);
  unsigned int offset = row_info->width - 1;

  for(size_t i = row_info->rowbytes - 1; i < row_info->rowbytes; i--, offset-=pixels_per_byte)
  {
    for(size_t j = 0, k = 0; j < 8; j+=row_info->bit_depth, k++)
      data[offset-k] = (data[i] >> j) & mask;
  }
}

void read_transform_fn(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
  if(row_info->bit_depth < 8)
    expand_to_byte(png_ptr, row_info, data);
}

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
  *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL, NULL, NULL);
  if (!png_ptr)
  {
    fprintf(stderr, "Error creating libpng read struct\n");
    return 0;
  }

  //Get the first info struct
  *info_ptr = png_create_info_struct(*png_ptr);

  if (!info_ptr)
  {
    png_destroy_read_struct(png_ptr, png_infopp_NULL, png_infopp_NULL);
    fprintf(stderr, "Error creating first info struct\n");
    return 0;
  }

  //Get the end info struct
  *end_info = png_create_info_struct(*png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(png_ptr, info_ptr, png_infopp_NULL);
    fprintf(stderr, "Error creating second info struct\n");
    return 0;
  }

  //Initialize IO and tell libpng we used bytes to check the header
  png_init_io(*png_ptr, fp);
  png_set_sig_bytes(*png_ptr, HEADER_BYTES);

  //Read file information
  png_read_info(*png_ptr, *info_ptr);

  //Set User transform functions
  png_set_read_user_transform_fn(*png_ptr, read_transform_fn);
  png_set_user_transform_info(*png_ptr, png_get_user_transform_ptr(*png_ptr), 8, png_get_channels(*png_ptr, *info_ptr));
  png_read_update_info(*png_ptr, *info_ptr);

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
