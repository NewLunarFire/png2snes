#ifndef TILE_H
#define TILE_H

uint8_t* convert_tiles(png_structp png_ptr, png_infop info_ptr, unsigned int bitplane_count, unsigned int tilesize, uint* data_size);
void output_tiles_binary(char* basename, uint8_t* data, int bytes);
void output_tiles_wla(char* basename, uint8_t* data, int bytes);

#endif
