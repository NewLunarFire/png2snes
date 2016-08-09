#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t* convert_to_bitplanes(uint8_t* tile, int bitplane_count);

void output_tiles_binary(char* basename, uint8_t* data, int bytes)
{
  FILE* fp;

  char* filename = (char*)malloc((strlen(basename) + 5) * sizeof(char));
  strcpy(filename, basename);
  strcat(filename, ".vra");

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
    char* filename = (char*)malloc((strlen(basename) + 11) * sizeof(char));
    strcpy(filename, basename);
    strcat(filename, "_vram.asm");
    fp = fopen(filename, "w");
  }

  if(!fp)
  {
    fprintf(stderr, "Could not open %s\n to write VRAM data", filename);
  }
  else
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

  if(fp != stdout && fp != NULL)
    fclose(fp);
}

uint8_t* get_tile(uint8_t* pixels, uint width, uint height, uint x, uint y)
{
  static uint8_t tile[64];

  for(size_t k = 0; k < 8; k++)
  {
      uint row = (y * 8) + k;
      memcpy(tile + (k*8), pixels + (row*width) + (x * 8), sizeof(uint8_t) * 8);
  }

  return tile;
}

uint8_t* convert_to_tiles_16_16(uint8_t* pixels, uint width, uint height, uint bitplane_count, uint* data_size)
{
  //Keep tracks of outputed bytes
  uint horizontal_tiles  = width / 16;
  uint vertical_tiles = height / 16;
  uint tile_count = horizontal_tiles * vertical_tiles;
  uint bytes_per_tile = 8 * bitplane_count;

  uint position;
  uint8_t *tile, *bitplanes;

  //Optimize this calculation. It shouldn't be that complex
  *data_size = ((((tile_count/8) + 1) * 8) + tile_count) * 16 * bitplane_count;
  uint8_t* data = (uint8_t*)malloc(*data_size * sizeof(uint8_t));

  //For each tile row
  for(size_t i = 0, k = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++, k++)
    {
      uint blerg = (k & 0x07) << 4;

      position = blerg * bytes_per_tile;
      tile = get_tile(pixels, width, height, 2*j, 2*i);
      bitplanes = convert_to_bitplanes(tile, bitplane_count);
      memcpy(data + position, bitplanes, bytes_per_tile);

      position = (blerg+1) * bytes_per_tile;
      tile = get_tile(pixels, width, height, (2*j) + 1, 2*i);
      bitplanes = convert_to_bitplanes(tile, bitplane_count);
      memcpy(data + position, bitplanes, bytes_per_tile);

      position = (blerg+16) * bytes_per_tile;
      tile = get_tile(pixels, width, height, 2*j, (2*i) + 1);
      bitplanes = convert_to_bitplanes(tile, bitplane_count);
      memcpy(data + position, bitplanes, bytes_per_tile);

      position = (blerg+17) * bytes_per_tile;
      tile = get_tile(pixels, width, height, (2*j) + 1, (2*i) + 1);
      bitplanes = convert_to_bitplanes(tile, bitplane_count);
      memcpy(data + position, bitplanes, bytes_per_tile);
    }
  }

  return data;
}

uint8_t* convert_to_tiles_8_8(uint8_t* pixels, uint width, uint height, uint bitplane_count, uint* data_size)
{
  //Keep tracks of outputed bytes
  uint horizontal_tiles  = width / 8;
  uint vertical_tiles = height / 8;
  uint bytes_per_tile = 8 * bitplane_count;

  uint position;
  uint8_t *tile, *bitplanes;

  *data_size = horizontal_tiles * vertical_tiles * 8 * bitplane_count;

  uint8_t* data = (uint8_t*)malloc(*data_size * sizeof(uint8_t));

  //For each tile row
  for(size_t i = 0, k = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++, k++)
    {
      tile = get_tile(pixels, width, height, j, i);
      bitplanes = convert_to_bitplanes(tile, bitplane_count);
      position = ((i * horizontal_tiles) + j) * bytes_per_tile;

      memcpy(data + position, tile, bytes_per_tile);
    }
  }

  return data;
}

uint8_t* convert_to_bitplanes(uint8_t* tile, int bitplane_count)
{
  uint subtile_size = 16;
  uint datasize = bitplane_count * 8;

  uint8_t* bitplanes = (uint8_t*)malloc(sizeof(uint8_t) * datasize);
  memset(bitplanes, 0, sizeof(uint8_t) * datasize);

  for(size_t row = 0; row < 8; row++)
  {
    for(size_t col = 0; col < 8; col++)
    {
      //Calculate number of bits to shift right
      uint bit = 7 - col;

      //Get color number
      uint8_t color = tile[(row*8) + col];
      uint offset = (row * 2) + (subtile_size * (bitplane_count / 2));

      switch(bitplane_count)
      {
        case 8:
          offset -= subtile_size;
          bitplanes[offset] |= ((color & 0x40) >> 6) << bit;
          bitplanes[offset+1] |= ((color & 0x80) >> 7) << bit;

          offset -= subtile_size;
          bitplanes[offset] |= ((color & 0x10) >> 4) << bit;
          bitplanes[offset+1] |= ((color & 0x20) >> 5) << bit;
        case 4:
          offset -= subtile_size;
          bitplanes[offset] |= ((color & 0x04) >> 2) << bit;
          bitplanes[offset+1] |= ((color & 0x08) >> 3) << bit;
        case 2:
          offset -= subtile_size;
          bitplanes[offset+1] |= (color & 0x01) << bit;
          bitplanes[offset] |= ((color & 0x02) >> 1) << bit;
      }

      //Store on bitplanes
      //for(size_t k = 0; k < bitplane_count; k++)
      //{
        //uint offset = (row*2) + ((k/2) * subtile_size) + (k & 0x01);
        //bitplanes[offset] |= (((color & (1 << k))) >> k) << bit;
      //}
    }
  }

  return bitplanes;
}
