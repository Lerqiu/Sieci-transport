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
#include "Repository.h"
#include "Socket.h"

static int sockfd = -1;
static struct sockaddr_in *addr = NULL;

void Socket_init(struct sockaddr_in *_addr)
{
    assert(sockfd == -1 && addr == NULL);

    int _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sockfd = _sockfd;
    addr = _addr;
}

void Socket_free()
{
    free(addr);
    close(sockfd);
    addr = NULL;
    sockfd = -1;
}

void _send(unsigned start, unsigned size)
{
    assert(sockfd != -1 && addr != NULL);

    char data[30];
    sprintf(data, "GET %u %u\n", start, size);
    int dataLen = strlen(data) + 1;

    if (sendto(sockfd, data, dataLen, 0, (struct sockaddr *)addr, sizeof(*addr)) != dataLen)
    {
        fprintf(stderr, "sendto error %s\n", strerror(errno));
        exit(1);
    }
}

void Socket_send(unsigned start, unsigned size)
{
    _send(start, size);
    Repository_addRequest(start, size);
}

void Socket_resend(Record *record)
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

    Repository_addResponse(buffer);
}

void Socket_receive(unsigned timout)
{
    assert(sockfd != -1 && addr != NULL);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

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
