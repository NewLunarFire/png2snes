#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t* convert_to_bitplanes(uint8_t* tile, int bitplane_count, int tilesize);

uint8_t* convert_to_tiles(uint8_t* data, uint width, uint height, uint bitplane_count, uint tilesize, uint* data_size)
{
  //Keep tracks of outputed bytes
  unsigned int horizontal_tiles  = width / 8;
  unsigned int vertical_tiles = height / 8;

  unsigned int output_bytes = horizontal_tiles * vertical_tiles * tilesize * tilesize * bitplane_count / 8;
  uint8_t* tile = (uint8_t*)malloc(tilesize * tilesize * sizeof(uint8_t));
  uint8_t** tiles = (uint8_t**)malloc(horizontal_tiles * vertical_tiles * sizeof(uint8_t*));

  //For each tile row
  for(size_t i = 0; i < vertical_tiles; i++)
  {
    //For each tile
    for(size_t j = 0; j < horizontal_tiles; j++)
    {
      for(size_t k = 0; k < tilesize; k++)
      {
        for(size_t l = 0; l < tilesize; l++)
        {
          int row = (i * tilesize) + k;
          int col = (j * tilesize) + l;

          tile[(k*tilesize)+l] = data[(row * width) + col];
        }
      }

      uint8_t* bitplanes = convert_to_bitplanes(tile, bitplane_count, tilesize);
    }
  }
}

uint8_t* convert_to_bitplanes(uint8_t* tile, int bitplane_count, int tilesize)
{
  uint datasize = bitplane_count * tilesize * tilesize / 8;
  uint subtile_size = tilesize * tilesize / 4;
  uint8_t* bitplanes = (uint8_t*)malloc(sizeof(uint8_t) * datasize);

  for(size_t i = 0; i < (bitplane_count * tilesize * tilesize / 8); i++)
    bitplanes[i] = 0;

  for(size_t row = 0; row < tilesize; row++)
  {
    for(size_t col = 0; col < tilesize; col++)
    {
      //Calculate number of bits to shift right
      uint bit = tilesize - col - 1;
      uint plane = (row*bitplane_count);

      //Expand to 8-bits palette index
      uint8_t color = tile[(row*tilesize) + col];

      //Store on bitplanes
      for(size_t k = 0; k < bitplane_count; k++)
      {
        uint offset = (row*2) + ((k/2) * subtile_size) + (k & 0x01);
        bitplanes[offset] |= (((color & (1 << k))) >> k) << bit;
      }
    }
  }

  for(size_t i = 0; i < (bitplane_count * tilesize * tilesize / 8); i++)
    printf("$%02X, ", bitplanes[i]);

  printf("\n");
  return bitplanes;
}
