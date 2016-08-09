#ifndef TILE_H
#define TILE_H

uint8_t* convert_to_tiles_16_16(uint8_t* pixels, uint width, uint height, uint bitplane_count, uint* data_size);
uint8_t* convert_to_tiles_8_8(uint8_t* pixels, uint width, uint height, uint bitplane_count, uint* data_size);
void output_tiles_binary(char* basename, uint8_t* data, int bytes);
void output_tiles_wla(char* basename, uint8_t* data, int bytes);

#endif
