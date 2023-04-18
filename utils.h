#ifndef UTILS_H
#define UTILS_H
FILE* open_file_for_reading(char* filename);
FILE* open_file_for_writing(char* filename);
long get_file_size(FILE* file);
void save_buffer_to_file(char* buff, char* filename);
void read_file_to_buffer(char* buff, char* filename);
char* get_file_info(char* filename);
#endif
