#include <stdio.h>
#include <stdlib.h>

//#include "argparser.h"
//#include "main.h"
//#include "palette.h"
//#include "pngfunctions.h"
//#include "tile.h"

int test1();
int test2();

struct unit_test_t {
  char* name;
  int (*testFunction)();
};

struct unit_test_t unitTests[] = {
  {"Passing test", test1},
  {"Failing test", test2},
  {NULL, NULL}
};

int main(int argc, char* argv[]) {
  unsigned int i = 0;
  unsigned int testsPassed = 0;
  unsigned int testsFailed = 0;

  printf("PNG2SNES test suite\n\n");
  for(i = 0; unitTests[i].name != NULL; i++) {
    printf("=====================================\n");
    printf("Test %d: %s\n", i+1, unitTests[i].name);
    printf("=====================================\n");
    int result = unitTests[i].testFunction();

    printf("=====================================\n");
    if(result != 0) {
      printf("Failed, returned %d exit status\n", result);
      testsFailed++;
    } else {
      printf("Success\n");
      testsPassed++;
    }
    printf("=====================================\n");
  }

  printf("Stats: %d run, %d passed, %d failed\n", i, testsPassed, testsFailed);
  return testsFailed > 1;
}

int test1() {
  printf("I am a test that passes\n");
  return 0;
}

int test2() {
  printf("I am a test that fails\n");
  return 1;
}
