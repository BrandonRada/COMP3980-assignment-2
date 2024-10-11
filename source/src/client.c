//
// Created by brandonr on 10/10/24.
//


#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define INPUT_FIFO "./input_fifo"
#define OUTPUT_FIFO "./output_fifo"

void usage(const char *program_name)__attribute__((noreturn));

void parse_arguments(int argc, char *argv[], char **filter, char **string);

int main(int argc, char *argv[])
{
    int    input_fd;
    int    output_fd;
    char   request[BUFFER_SIZE];
    char   response[BUFFER_SIZE];
    size_t bytes_read;
    char  *filter = NULL;
    char  *string = NULL;

    parse_arguments(argc, argv, &filter, &string);

    sprintf(request, "%s:%s", filter, string);

    input_fd = open(INPUT_FIFO, O_WRONLY | O_CLOEXEC);
    if(input_fd == -1)
    {
        perror("Error opening input FIFO");
        exit(EXIT_FAILURE);
    }
    write(input_fd, request, strlen(request));
    close(input_fd);

    output_fd = open(OUTPUT_FIFO, O_RDONLY | O_CLOEXEC);
    if(output_fd == -1)
    {
        perror("Error opening output FIFO");
        exit(EXIT_FAILURE);
    }
    bytes_read = (size_t)read(output_fd, response, BUFFER_SIZE - 1);
    if(bytes_read > 0)
    {
        response[bytes_read] = '\0';
        printf("Response from server: %s\n", response);
    }
    close(output_fd);

    return 0;
}



void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s -f <upper|lower|null> -s <string>\n", program_name);
    exit(EXIT_FAILURE);
}

void parse_arguments(int argc, char *argv[], char **filter, char **string)
{
    int opt;
    opterr = 0;
    while((opt = getopt(argc, argv, "f:s:")) != -1)
    {
        switch(opt)
        {
            case 'f':
                *filter = optarg;
                break;
            case 's':
                *string = optarg;
                break;
            default:
                usage(argv[0]);
        }
    }
    if(!*filter || !*string)
    {
        usage(argv[0]);
    }
}
