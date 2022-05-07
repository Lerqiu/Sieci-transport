#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "Main.h"

static void _correctInputAmount(int argc)
{
    if (argc != 5)
    {
        fprintf(stderr, "Incorrect input amount!\n");
        exit(EXIT_FAILURE);
    }
}

struct sockaddr_in *Input_getAddr(int argc, char *argv[])
{
    _correctInputAmount(argc);

    char *stringIP = argv[1];
    char *stringPort = argv[2];

    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    assert(addr != NULL);
    bzero(addr, sizeof(*addr));
    addr->sin_family = AF_INET;

    if (inet_pton(AF_INET, stringIP, &(addr->sin_addr)) != 1)
    {
        fprintf(stderr, "Wrong ip format %s\n", stringIP);
        exit(EXIT_FAILURE);
    }

    uint16_t port;
    if (sscanf(stringPort, "%hu", &port) != 1 && port == 0)
    {
        fprintf(stderr, "Wrong port %u %s\n", port, stringPort);
        exit(EXIT_FAILURE);
    }
    addr->sin_port = htons(port);

    return addr;
}

unsigned Input_getSize(int argc, char *argv[])
{
    _correctInputAmount(argc);
    unsigned size = 0;
    char *stringSize = argv[4];

    if (sscanf(stringSize, "%u", &size) != 1 || (PACKAGE_MAX_START + PACKAGE_MAX_LEN < size) || size == 0)
    {
        fprintf(stderr, "Wrong size %u %s\n", size, stringSize);
        exit(EXIT_FAILURE);
    }
    return size;
}

char *Input_getFilePath(int argc, char *argv[])
{
    _correctInputAmount(argc);
    char *path = argv[3];
    int len = strlen(path);

    if (len == 0)
    {
        fprintf(stderr, "Wrong size %u %s\n", len, path);
        exit(EXIT_FAILURE);
    }
    return path;
}