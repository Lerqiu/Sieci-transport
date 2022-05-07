#ifndef MY_REPOSITORY
#define MY_REPOSITORY

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

void Repository_flush();
unsigned Repository_freeSpace();

void Repository_addResponse(u_int8_t *buffor);
void Repository_addRequest(unsigned start, unsigned size);
bool Repository_isFree();

void Repository_forEach(void (*func)(Record *));

#endif