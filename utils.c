#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

FILE* open_file_for_reading(char* filename) {
  return fopen(filename, "rb");
}

FILE* open_file_for_writing(char* filename) {
  return fopen(filename, "w+");
}

long get_file_size(FILE* file) {
  if ((file == NULL) ||
      (fseek(file, 0, SEEK_END) < 0)) {
    return -1;
  }

  long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return size;
}

int save_buffer_to_file(char* buff, char* filename) {
  FILE *fp = open_file_for_writing(filename);
  int size = strlen(buff);
  int res1 = fwrite(buff, size, 1, fp);
  printf("fwrite size: %d\n", res1);
  if(res1 > 0) {
    fflush(fp);
  }
  return res1;
}

void read_file_to_buffer(char* buff, char* filename) {
  FILE *fp = open_file_for_reading(filename);
  int size = get_file_size(fp);
  fread(buff, size, 1, fp);
  int x = 5;
}

char* get_file_info(char* filename) {
  FILE *fp = open_file_for_reading(filename);
  return (char*) get_file_size(fp);
}
