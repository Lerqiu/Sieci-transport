#pragma once

#include <aio.h>
#include "Repository.h"

void Socket_init(struct sockaddr_in *);
void Socket_free();

void Socket_send(Record *);
void Socket_receive(struct timeval);