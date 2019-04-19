#define DEFAULT_TIME_BUF_SIZE 256

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

FILE* log_file_pointer;
char* log_path;

void get_curr_time(char* time_buf);
void init_log(char* fullpath);
void close_log();
void print_log(char* format, ...);