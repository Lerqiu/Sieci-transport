#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "Main.h"
#include "Socket.h"
#include "Manager.h"

static int sockfd = -1;

void Socket_init(struct sockaddr_in *addr)
{
    assert(sockfd == -1);

    int _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (connect(_sockfd, (struct sockaddr *)addr, sizeof(*addr)))
    {
        fprintf(stderr, "connection error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sockfd = _sockfd;
    free(addr);
}

void Socket_free()
{
    close(sockfd);
    sockfd = -1;
}

static void _send(unsigned start, unsigned size)
{
    assert(sockfd != -1);

    char data[30];
    if(sprintf(data, "GET %u %u\n", start, size) < 7)
    {
        fprintf(stderr, "sprintf error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int dataLen = strlen(data) + 1;

    if (send(sockfd, data, dataLen, 0) != dataLen)
    {
        fprintf(stderr, "send error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void Socket_send(Record *record)
{
    _send(record->start, record->size);
}

static void _receive()
{
    u_int8_t buffer[IP_MAXPACKET + 1];
    assert(buffer != NULL);

    ssize_t datagram_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, NULL, NULL);
    if (datagram_len < 0)
    {
        fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    Manager_manageResponse(buffer);
}

void Socket_receive(struct timeval timeout)
{
    assert(sockfd != -1);
    while (timeout.tv_sec > 0 || timeout.tv_usec > 0)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int selectResult = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        assert(selectResult >= 0);

        if (FD_ISSET(sockfd, &readfds))
        {
            _receive();
            return;
        }
    }
}
