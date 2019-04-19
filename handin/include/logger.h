#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *log_file_path;

int init_log();
void logger(const char *fmt, ...);
void dump_log(const char *fmt, ...);
void close_log();
