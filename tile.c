#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tile.h"

void output_tiles_binary(char* basename, uint8_t* data, int bytes)
{
  FILE* fp;
  char* filename;

  asprintf(&filename, "%s.vra", basename);
  fp = fopen(filename, "wb");

  if(!fp)
  {
    fprintf(stderr, "Could not open %s\n to write VRAM data", filename);
  }
  else
  {
    for(size_t i = 0; i < bytes; i++)
      putc(data[i], fp);
  }
}

void output_tiles_wla(char* basename, uint8_t* data, int bytes)
{
  FILE* fp;
  char* filename;

  if(strcmp(basename, "-") == 0)
    fp = stdout;
  else
  {
    asprintf(&filename, "%s_vram.asm", basename);
    fp = fopen(filename, "w");
  }

  if(fp)
  {
    for(size_t i = 0; i < bytes; i++)
    {
      if((i & 0x0F) == 0)
        fprintf(fp, "\n\t.db ");

      fprintf(fp, "$%02X", data[i]);

      if(((i & 0x0F) < 15) && (i < (bytes-1)))
          fprintf(fp, ", ");
    }

    fprintf(fp, "\n");
  }
  else
    fprintf(stderr, "Could not open %s\n to write VRAM data", filename);

  if(fp != stdout && fp != NULL)
    fclose(fp);
}

void print_tile(uint8_t* tile) {
  size_t i, j;
  for(i = 0; i < 8; i++) {
    for(j = 0; j < 8; j++) {
      printf("%d", tile[(i*8) + j]);
    }

    printf("\n");
  }

  printf("\n");
}

void print_bitplanes(uint8_t* bitplanes) {
    size_t i = 0;
    for(i = 0; i < (SUBTILE_SIZE - 1); i++) {
      printf("%02X, ", bitplanes[i]);
    }

    printf("%02X\n", bitplanes[SUBTILE_SIZE - 1]);
}

uint8_t* get_tile_from_png(uint8_t* destination, png_structp png_ptr, png_bytepp row_pointers, int x, int y)
{
  unsigned int row = y * 8;

  for(size_t k = 0; k < 8; k++, row++)
      memcpy(destination + (k*8), row_pointers[row] + (x*8), 8);

  return destination;
}

void convert_to_bitplanes(uint8_t* destination, const uint8_t* source, int bitplane_count)
{
  uint8_t color;

  memset(destination, 0, bitplane_count * 8);

  //For each row
  for(size_t row = 0, offset = 0; row < 8; row++, offset+=2)
  {
    //For each column
    for(size_t bit = 7; bit < 8; bit--)
    {
      //Get color number
      color = source[(row*8) + 7 - bit];

      switch(bitplane_count)
      {
        case 8:
          destination[offset+(SUBTILE_SIZE*3)+1] |= ((color & 0x80) >> 7) << bit;
          destination[offset+(SUBTILE_SIZE*3)] |= ((color & 0x40) >> 6) << bit;
          destination[offset+(SUBTILE_SIZE*2)+1] |= ((color & 0x20) >> 5) << bit;
          destination[offset+(SUBTILE_SIZE*2)] |= ((color & 0x10) >> 4) << bit;
        case 4:
          destination[offset+SUBTILE_SIZE+1] |= ((color & 0x08) >> 3) << bit;
          destination[offset+SUBTILE_SIZE] |= ((color & 0x04) >> 2) << bit;
        case 2:
          destination[offset+1] |= ((color & 0x02) >> 1) << bit;
          destination[offset] |= (color & 0x01) << bit;
      }
    }
  }
}

uint8_t* convert_to_tiles_16_16(png_structp png_ptr, png_infop info_ptr, unsigned int bitplane_count, unsigned int* data_size)
{
  //Keep tracks of outputed bytes
  //Get image size and bit depth
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  //unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  unsigned int horizontal_tiles  = width / 16;
  unsigned int vertical_tiles = height / 16;
  unsigned int tile_count = horizontal_tiles * vertical_tiles;
  unsigned int bytes_per_tile = 8 * bitplane_count;

  unsigned int tile_number;
  uint8_t tile[TILE_SIZE];
  uint8_t* data;

  png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);
  for(size_t i = 0; i < height; i++)
    row_pointers[i] = malloc(sizeof(png_byte) * rowbytes);

  png_read_image(png_ptr, row_pointers);

  //Optimize this calculation. It shouldn't be that complex
  *data_size = ((((tile_count/8) + 1) * 8) + tile_count) * 16 * bitplane_count;

  //Allocate required space
  data = malloc(*data_size);

  //For each tile row
  for(size_t i = 0, k = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++, k++)
    {
      //Tile
      tile_number = ((k & 0x07) << 1) | ((k & ~(0x07)) << 2);
      get_tile_from_png(tile, png_ptr, row_pointers, 2*j, 2*i);
      convert_to_bitplanes(data + (tile_number * bytes_per_tile), tile, bitplane_count);

      //print_tile(tile);
      //print_bitplanes(data + (tile_number * bytes_per_tile));
      printf("Tile Number = %u\nOffset = %u\n", tile_number, tile_number * bytes_per_tile);

      //Tile + 1
      tile_number += 1;
      get_tile_from_png(tile, png_ptr, row_pointers, (2*j) + 1, 2*i);
      convert_to_bitplanes(data + (tile_number * bytes_per_tile), tile, bitplane_count);

      //print_tile(tile);
      //print_bitplanes(data + (tile_number * bytes_per_tile));
      printf("Tile Number = %u\nOffset = %u\n", tile_number, tile_number * bytes_per_tile);

      //Tile + 16
      tile_number += 15;
      get_tile_from_png(tile, png_ptr, row_pointers, 2*j, (2*i) + 1);
      convert_to_bitplanes(data + (tile_number * bytes_per_tile), tile, bitplane_count);

      //print_tile(tile);
      //print_bitplanes(data + (tile_number * bytes_per_tile));
      printf("Tile Number = %u\nOffset = %u\n", tile_number, tile_number * bytes_per_tile);

      //Tile + 17
      tile_number += 1;
      get_tile_from_png(tile, png_ptr, row_pointers, (2*j) + 1, (2*i) + 1);
      convert_to_bitplanes(data + (tile_number * bytes_per_tile), tile, bitplane_count);

      //print_tile(tile);
      //print_bitplanes(data + (tile_number * bytes_per_tile));
      printf("Tile Number = %u\nOffset = %u\n", tile_number, tile_number * bytes_per_tile);
    }
  }

  return data;
}

uint8_t* convert_to_tiles_8_8(png_structp png_ptr, png_infop info_ptr, unsigned int bitplane_count, unsigned int* data_size)
{
  //Keep tracks of outputed bytes
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  unsigned int horizontal_tiles  = width / 8;
  unsigned int vertical_tiles = height / 8;
  unsigned int bytes_per_tile = 8 * bitplane_count;

  //unsigned int position;
  uint8_t *tile, *data;

  png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);
  for(size_t i = 0; i < height; i++)
    row_pointers[i] = malloc(sizeof(png_byte) * rowbytes);

  png_read_image(png_ptr, row_pointers);

  *data_size = horizontal_tiles * vertical_tiles * 8 * bitplane_count;
  data = malloc(*data_size);
  tile = malloc(TILE_SIZE);

  //For each tile row
  for(size_t i = 0, k = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++, k++)
    {
      get_tile_from_png(tile, png_ptr, row_pointers, j, i);
      convert_to_bitplanes(data + (k * bytes_per_tile), tile, bitplane_count);
    }
  }

  return data;
}

uint8_t* convert_tiles(png_structp png_ptr, png_infop info_ptr, unsigned int bitplane_count, unsigned int tilesize, unsigned int* data_size)
{
  /*

  //Keep tracks of outputed bytes
  //Get image size and bit depth
  unsigned int height = png_get_image_height(png_ptr, info_ptr);
  unsigned int width = png_get_image_width(png_ptr, info_ptr);
  unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  unsigned int horizontal_tiles  = width / tilesize;
  unsigned int vertical_tiles = height / tilesize;
  unsigned int tile_count = horizontal_tiles * vertical_tiles;
  unsigned int bytes_per_tile = 8 * bitplane_count;


  png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);
  for(size_t i = 0; i < height; i++)
    row_pointers[i] = malloc(sizeof(png_byte) * rowbytes);

  png_read_image(png_ptr, row_pointers);

  //For each tile row
  for(size_t i = 0, k = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++, k++)
    {
      //Convert single tile
    }
  }
  */

  if(tilesize == 8)
  {
    return convert_to_tiles_8_8(png_ptr, info_ptr, bitplane_count, data_size);
  }
  else// if(tilesize == 16)
  {
    return convert_to_tiles_16_16(png_ptr, info_ptr, bitplane_count, data_size);
  }
}
