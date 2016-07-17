#include <stdint.h>
#include <stdio.h>
#include <png.h>

#define HEADER_BYTES 8
int main(int argc, char *argv[])
{
  unsigned char header[8];
  int is_png;

  //Check if we have a sufficient number of arguments before continuing
  if(argc < 2)
  {
    printf("Error: Too few arguments");
    printf("Usage: png2snes file");
  }

  //Open the file
  FILE *fp = fopen(argv[1], "rb");
  if (!fp)
    return -1;

  //Read bytes at the beginning and verify if this is a PNG file
  fread(header, 1, HEADER_BYTES, fp);
  is_png = !png_sig_cmp(header, 0, HEADER_BYTES);

  if (!is_png)
  {
    fclose(fp);
    printf("File is not a PNG");
    return -2;
  }

  //Create the PNG Read Struct
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
  if (!png_ptr)
  {
    fclose(fp);
    printf("Error creating libpng read struct");
    return -4;
  }

  //Get the first info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    printf("Error creating first info struct");
    return -5;
  }

  //Get the end info struct
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    printf("Error creating second info struct");
    return -6;
  }

  //Initialize IO and tell libpng we used bytes to check the header
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, HEADER_BYTES);

  //Read file information
  png_read_info(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);

  //Verify that this is a paletted PNG image
  if(color_type != PNG_COLOR_TYPE_PALETTE)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    printf("File is not paletted");
    return -7;
  }

  //Fetch Palette from PNG file
  png_color* palette;
  int num_palette;
  png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

  printf("Palette contains %d colors\n", num_palette);
  printf("CGRAM section is %d bytes long\n", num_palette * 2);

  //Start outputting data in WLA-DX format
  printf(".dw ");

  for(unsigned int i = 0; i < num_palette; i++)
  {
    uint16_t nes_color = ((palette[i].blue & 0xF8) << 7) | ((palette[i].green & 0xF8) << 2) | (palette[i].red >> 3);
    printf("%04X", nes_color);

    if(i < (num_palette - 1))
      printf(", ");
  }

  printf("\n");

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(fp);
  return 0;
}
