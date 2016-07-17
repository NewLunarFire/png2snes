#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <png.h>

#define HEADER_BYTES 8
#define CONVERT_TO_SNES_COLOR(red, green, blue) (((blue & 0xF8) << 7) | ((green & 0xF8) << 2) | (red >> 3))

void close_file_exit(FILE* fp, int exit_code);
void close_file_and_png_exit(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info, int exit_code);

int detect_png(FILE* fp);
int initialize_libpng(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info);
int detect_palette();
void convert_palette(png_structp png_ptr, png_infop info_ptr);

void close_file_exit(FILE* fp, int exit_code)
{
  fclose(fp);
  exit(exit_code);
}

void close_file_and_png_exit(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info, int exit_code)
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

  //Check if we have a sufficient number of arguments before continuing
  if(argc < 2)
  {
    printf("Error: Too few arguments");
    printf("Usage: png2snes file");
  }

  //Open the file
  fp = fopen(argv[1], "rb");
  if (!fp)
    return -1;

  //If file is not a PNG, quit
  if (!detect_png(fp))
    close_file_exit(fp, -2);

  //If initializing libpng failed, quit
  if(!initialize_libpng(fp, &png_ptr, &info_ptr, &end_info))
    close_file_exit(fp, -3);

  //Verify that this is a paletted PNG image
  if(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_PALETTE)
  {
    printf("File does not contain a palette");
    close_file_and_png_exit(fp, &png_ptr, &info_ptr, &end_info, -4);
  }

  convert_palette(png_ptr, info_ptr);
  close_file_and_png_exit(fp, &png_ptr, &info_ptr, &end_info, 0);

  return 0;
}

int detect_png(FILE* fp)
{
  unsigned char header[HEADER_BYTES];
  int is_png;

  //Read bytes at the beginning and verify if this is a PNG file
  fread(header, 1, HEADER_BYTES, fp);
  is_png = !png_sig_cmp(header, 0, HEADER_BYTES);

  if (!is_png)
    printf("File is not a PNG");

  return is_png;
}

int initialize_libpng(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info)
{
  //Create the PNG Read Struct
  *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
  if (!png_ptr)
  {
    printf("Error creating libpng read struct");
    return 0;
  }

  //Get the first info struct
  *info_ptr = png_create_info_struct(*png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    printf("Error creating first info struct");
    return 0;
  }

  //Get the end info struct
  *end_info = png_create_info_struct(*png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
    printf("Error creating second info struct");
    return 0;
  }

  //Initialize IO and tell libpng we used bytes to check the header
  png_init_io(*png_ptr, fp);
  png_set_sig_bytes(*png_ptr, HEADER_BYTES);

  //Read file information
  png_read_info(*png_ptr, *info_ptr);
}

void convert_palette(png_structp png_ptr, png_infop info_ptr)
{
  png_color* palette;
  int num_palette;

  //Fetch Palette from PNG file
  png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

  printf("Palette contains %d colors\n", num_palette);
  printf("CGRAM section is %d bytes long\n", num_palette * 2);

  //Start outputting data in WLA-DX format
  printf(".dw ");

  for(unsigned int i = 0; i < num_palette; i++)
  {
    uint16_t nes_color = CONVERT_TO_SNES_COLOR(palette[i].red, palette[i].green, palette[i].blue);
    printf("%04X", nes_color);

    if(i < (num_palette - 1))
      printf(", ");
  }

  printf("\n");
}
