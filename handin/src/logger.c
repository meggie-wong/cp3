#include "logger.h"
// #define DEBUG_VERBOSE 1
FILE *log_file;
/*
 * Initiate log file.
 * Return 0 on success, return 1 on failure.
 */
int init_log(char* log_file_path) {
  log_file = fopen(log_file_path, "w");
  if (log_file == NULL) {
    return -1;
  }
  return 0;
}

/*
 * Write log to console.
 */
void logger(const char *fmt, ...){
  va_list args;
  va_start(args, fmt);
  vfprintf(log_file, fmt, args);
  va_end(args);
  fflush(log_file);
}

/*
 * Dump log into log_file given by comman-line argument
 */
void dump_log(const char *fmt, ...){
  va_list args;
  va_start(args, fmt);
  vfprintf(log_file, fmt, args);
  va_end(args);
  fprintf(log_file, "\n");
  fflush(log_file);
}

/*
 * Close log file.
 */
void close_log() {
  fclose(log_file);
}

