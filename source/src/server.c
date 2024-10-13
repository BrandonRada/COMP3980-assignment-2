//
// Created by brandonr on 10/10/24.
//

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define SLEEP_LENGTH 1
#define INPUT_FIFO "./input_fifo"
#define OUTPUT_FIFO "./output_fifo"

typedef char (*filter_function)(char);

static char upper_filter(char c);

static char lower_filter(char c);

static char null_filter(char c);

filter_function get_filter_function(const char *filter_name);

void *handle_client(void *arg);

void handle_sigInt(int sig) __attribute__((noreturn));

int main(void)
{
    int       input_fd;
    pthread_t thread;
    char     *request;

    if(mkfifo(INPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1 && errno != EEXIST)
    {
        perror("Failed to create input FIFO");
        exit(EXIT_FAILURE);
    }
    if(mkfifo(OUTPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1 && errno != EEXIST)
    {
        perror("Failed to create output FIFO");
        unlink(INPUT_FIFO);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigInt);

    input_fd = open(INPUT_FIFO, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if(input_fd == -1)
    {
        perror("Failed to open input FIFO\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    while(true)
    {
        char    buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer) - 1);

        if(bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            request            = strdup(buffer);

            pthread_create(&thread, NULL, handle_client, request);
            pthread_detach(thread);
        }
        else if(bytes_read == -1 && errno == EAGAIN)
        {
            sleep(SLEEP_LENGTH);
            continue;
        }
        else if(bytes_read == 0)
        {
            // No data to read at the moment, just wait
            sleep(SLEEP_LENGTH);
        }
        else
        {
            perror("Error reading from input FIFO");
            break;
        }
    }

    close(input_fd);
    return 0;
}

static char upper_filter(char c)
{
    return (char)toupper(c);
}

static char lower_filter(char c)
{
    return (char)tolower(c);
}

static char null_filter(char c)
{
    return c;
}

filter_function get_filter_function(const char *filter_name)
{
    if (filter_name == NULL)
    {
    	fprintf(stderr, "Filter name is NULL\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(filter_name, "upper") == 0)
    {
        return upper_filter;
    }
    if(strcmp(filter_name, "lower") == 0)
    {
        return lower_filter;
    }
    if(strcmp(filter_name, "null") == 0)
    {
        return null_filter;
    }

    fprintf(stderr, "Not a Valid Filter: %s\n", filter_name);
    exit(EXIT_FAILURE);
}

void *handle_client(void *arg)
{
    int             output_fd;
    char           *request = (char *)arg;
    char           *saveptr;
    const char     *filter_type = strtok_r(request, ":", &saveptr);
    char           *string      = strtok_r(NULL, ":", &saveptr);
    filter_function filter;

    if(!filter_type || !string)
    {
        fprintf(stderr, "Invalid request format\n");
        free(request);
        return NULL;
    }
    filter = get_filter_function(filter_type);

    for(int i = 0; string[i]; i++)
    {
        string[i] = filter(string[i]);
    }

    output_fd = open(OUTPUT_FIFO, O_WRONLY | O_CLOEXEC);
    if(output_fd == -1)
    {
        perror("Failed to open output file");
        free(request);
        return NULL;
    }
    //    printf("The filtered strings length: %zu\n", strlen(string));
    write(output_fd, string, strlen(string));
    close(output_fd);

    free(request);
    return NULL;
}

void handle_sigInt(int sig)
{
    const char *message;

    (void)sig;
    message = "\nShutting down server...";
    write(STDOUT_FILENO, message, strlen(message));
    unlink(INPUT_FIFO);
    unlink(OUTPUT_FIFO);
    _exit(EXIT_SUCCESS);
}
