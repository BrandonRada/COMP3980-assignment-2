//
// Created by brandonr on 10/10/24.
//

#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

#define INPUT_FIFO "./input_fifo"
#define OUTPUT_FIFO "./output_fifo"
#define BUFFER_SIZE 4096
#define SLEEP_LENGTH 1

typedef char (*filter_function)(char);

void           *handle_client(void *arg);
void            handle_sigInt(int sig) __attribute__((noreturn));
void            usage(const char *program_name);
void            parse_arguments(int argc, char *argv[], char **input_fifo, char **output_fifo);
filter_function get_filter_function(const char *filter_name);

#endif    // SERVER_H
