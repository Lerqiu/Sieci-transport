#ifndef MY_SOCKET
#define MY_SOCKET

#include "Repository.h"

void Socket_init(struct sockaddr_in *);
void Socket_free();
void Socket_send(unsigned start, unsigned len);
void Socket_receive(unsigned timout);
void Socket_resend(Record *record);

#endif