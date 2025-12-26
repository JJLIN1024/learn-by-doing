#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

void test_debug() {
  debug("This is the %s", "message");
  debug("Debug test message");
}

int test_check(char *file_name) {
  FILE *input = NULL;
  char *block = NULL;

  block = malloc(100);
  check_mem(block); // should work

  input = fopen(file_name, "r");
  check(input, "Failed to open %s.", file_name);

  free(block);
  fclose(input);
  return 0;

error:
  if (block)
    free(block);
  if (input)
    fclose(input);
  return -1;
}

int main(int argc, char **argv) {
  test_debug();

  check(argc == 2, "Need a arg");
  return 0;

error:
  return 1;
}
