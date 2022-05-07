#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "Output.h"
#include "Main.h"

static int filefd = -1;

void Output_init(char path[])
{
    assert(filefd == -1);

    int fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        fprintf(stderr, "Open error %d <->  %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    filefd = fd;
}

void Output_free()
{
    close(filefd);
    filefd = -1;
}

void Output_write(u_int8_t buffer[], unsigned size)
{
    assert(filefd != -1);
    int i;

    while (size - (i = write(filefd, buffer, size)) > 0)
    {
        size -= i;
        buffer += i;
    }
    if (i < 0)
    {
        fprintf(stderr, "Error - %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    assert(i != -1);
}
