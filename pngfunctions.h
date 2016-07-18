#ifndef PNG_FUNCTIONS_H
#define PNG_FUNCTIONS_H

  int detect_png(FILE* fp);
  int initialize_libpng(FILE* fp, png_structp* png_ptr, png_infop* info_ptr, png_infop* end_info);
  int detect_palette();

#endif //PNG_FUNCTIONS_H
