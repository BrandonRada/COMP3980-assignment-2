//
// Created by brandonr on 10/10/24.
//

#ifndef CLIENT_H
#define CLIENT_H

#define INPUT_FIFO "./input_fifo"
#define OUTPUT_FIFO "./output_fifo"
#define BUFFER_SIZE 4096

void usage(const char *program_name) __attribute__((noreturn));

void parse_arguments(int argc, char *argv[], char **filter, char **string);

#endif    // CLIENT_H
