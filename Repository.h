#pragma once

#include <netinet/ip.h>
#include <stdbool.h>

typedef struct
{
    unsigned start;
    unsigned size;
    u_int8_t *buffer;
} Record;

void Repository_init();
void Repository_free();

void Repository_addResponse(Record *);
void Repository_addRequest(Record *);

unsigned Repository_freeSpace();
bool Repository_isFree();