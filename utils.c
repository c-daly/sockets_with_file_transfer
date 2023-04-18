#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

long get_file_size(FILE* file) {
  if ((file == NULL) ||
      (fseek(file, 0, SEEK_END) < 0)) {
    return -1;
  }

  long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return size;
}
