#ifndef PALETTE_H
#define PALETTE_H
#include <stdint.h>

uint16_t* convert_palette(png_structp png_ptr, png_infop info_ptr, int* size, int pad_to_size);
void output_palette_binary(char* basename, uint16_t* data, int words);
void output_palette_wla(char* basename, uint16_t* data, int words);

#endif //PALETTE_H
