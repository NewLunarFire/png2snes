#ifndef ARG_PARSER_H
#define  ARG_PARSER_H

  /* Supplied arguments */
  struct arguments
  {
    int verbose;
    int binary;
    char *input_file;
    char *output_file;
    int bitplanes;
    int tilesize;
  };

  /* Argument parser */
  struct arguments parse_arguments(int argc, char **argv);

#endif //ARG_PARSER_H
