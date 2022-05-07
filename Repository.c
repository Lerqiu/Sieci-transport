#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include "Main.h"
#include "Repository.h"
#include "Output.h"

static Record **repoData = NULL;

void Repository_init()
{
    assert(repoData == NULL);

    repoData = malloc(sizeof(Record *));
    *repoData = NULL;
}

void Repository_free()
{
    assert(repoData != NULL);

    free(repoData);
    repoData = NULL;
}

static Record *_createRecordHeader(unsigned start, unsigned size)
{
    Record *record = malloc(sizeof(Record));
    assert(record != NULL);
    record->start = start;
    record->size = size;
    record->buffer = NULL;
    return record;
}

static Record *_bufforToRecord(u_int8_t buffor[])
{
    unsigned start, size, offset = 8;

    assert(sscanf(buffor, "DATA %u %u\n", &start, &size) == 2);
    while (buffor[offset++] != '\n' && offset < 100)
        ;
    assert(offset < 100);

    Record *record = _createRecordHeader(start, size);
    record->buffer = malloc(sizeof(u_int8_t) * size);
    for (unsigned dataIntex = 0; dataIntex < size; dataIntex++)
    {
        record->buffer[dataIntex] = buffor[dataIntex + offset];
    }

    return record;
}

void Repository_addResponse(u_int8_t *buffor)
{
    Record *record = _bufforToRecord(buffor);

    if (*repoData != NULL &&
        (*repoData)->start == record->start &&
        (*repoData)->size == record->size &&
        (*repoData)->buffer == NULL)
    {
        (*repoData)->buffer = record->buffer;
    }

    free(record);
}

void Repository_addRequest(unsigned start, unsigned size)
{
    Record *record = _createRecordHeader(start, size);
    assert(*repoData == NULL);
    *repoData = record;
}

unsigned Repository_freeSpace()
{
    if (*repoData == NULL)
        return 1;
    return 0;
}

void Repository_flush()
{
    if (*repoData != NULL && (*repoData)->buffer != NULL)
    {
        Output_write((*repoData)->buffer, (*repoData)->size);
        free((*repoData)->buffer);
        free(*repoData);
        *repoData = NULL;
    }
}

bool Repository_isFree()
{
    return *repoData == NULL;
}

void Repository_forEach(void (*func)(Record *))
{
    assert(repoData != NULL);
    if (*repoData != NULL)
        func(*repoData);
}