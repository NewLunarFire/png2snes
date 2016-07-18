#include <stdlib.h>
#include <argp.h>

#include "argparser.h"

/* Version and bugs address */
const char *argp_program_version = "png2snes beta";
const char *argp_program_bug_address = "<tommy.savaria@protonmail.ch>";

/* Program documentation. */
static char doc[] = "png2snes -- Create SNES Graphics from PNG files";

/* A description of the arguments we accept. */
static char args_doc[] = "INPUT_FILE";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce more verbose output" },
  {"quiet",    'q', 0,      0,  "Don't produce any output" },
  {"output",   'o', "FILE", 0, "Output to FILE instead of standard output" },
  {"bitplanes", 'b', "PLANES", 0, "Number of bitplanes to generate per tile (2, 4 or 8)"},
  {"tilesize", 't', "SIZE", 0, "Size of tiles (8x8 or 16x16) to generate"},
  { 0 }
};

int parse_number(char* number)
{
  char* endptr;
  int val = strtol(number, &endptr, 0);

  /* Check for various possible errors */
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
     return 0;

  if (endptr == number)
    return 0;

  /* Return valid conversion result */
  return (int)val;
}

int parse_tilesize(char* arg)
{
  int ts = parse_number(arg);

  /* Check for invalid values */
  if(ts != 8 && ts != 16)
    return 0;

  return ts;
}

int parse_bitplanes(char* arg)
{
  int bp = parse_number(arg);

  /* Check for invalid values */
  if(bp != 2 && bp != 4 && bp != 8)
    return 0;

  return bp;
}

/* Parse a single option. */
error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q': case 's':
      arguments->verbose = 0;
      break;
    case 'v':
      arguments->verbose++;
      break;
    case 'o':
      arguments->output_file = arg;
      break;
    case 'b':
      arguments->bitplanes = parse_bitplanes(arg);
      if(!arguments->bitplanes)
      {
        fprintf(stderr, "Invalid value for bitplanes: %s (Possible values are 2, 4 and 8)\n", arg);
        argp_usage(state);
      }
      break;
    case 't':
      arguments->tilesize = parse_tilesize(arg);
      if(!arguments->tilesize)
      {
        fprintf(stderr, "Invalid value for bitplanes: %s (Possible values are 8 and 16)\n", arg);
        argp_usage(state);
      }
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);

      arguments->input_file = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

struct arguments parse_arguments(int argc, char **argv)
{
  struct arguments arguments;

  /* Default values. */
  arguments.verbose = 1;
  arguments.output_file = "-";
  arguments.bitplanes = 0;
  arguments.tilesize = 0;

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  return arguments;
}
