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
#include <sys/queue.h>
#include "Main.h"
#include "Repository.h"
#include "Output.h"
#include "Manager.h"

typedef struct rRecord
{
    TAILQ_ENTRY(rRecord)
    entries;
    Record *record;
} RepositoryRecord;

TAILQ_HEAD(RepoQueue, rRecord);
static struct RepoQueue rQueue;
static unsigned freeSpace = REPOSITORY_SIZE;

void Repository_init()
{
    TAILQ_INIT(&rQueue);
}

void Repository_free()
{
    while (!TAILQ_EMPTY(&rQueue))
        TAILQ_REMOVE(&rQueue, TAILQ_FIRST(&rQueue), entries);
}

static void _flushConsistentData()
{
    RepositoryRecord *r;
    while (!TAILQ_EMPTY(&rQueue) && (r = TAILQ_FIRST(&rQueue))->record->buffer != NULL)
    {
        TAILQ_REMOVE(&rQueue, r, entries);
        Output_write(r->record->buffer, r->record->size);
        free(r->record->buffer);
        free(r->record);
        free(r);
        freeSpace++;
        Manager_sendRestPakages();
    }
}

void Repository_addResponse(Record *record)
{
    RepositoryRecord *r;
    TAILQ_FOREACH(r, &rQueue, entries)
    if (r->record->start == record->start &&
        r->record->size == record->size &&
        r->record->buffer == NULL)
    {
        r->record->buffer = record->buffer;
        free(record);
        _flushConsistentData();
        return;
    }
}

void Repository_addRequest(Record *record)
{
    RepositoryRecord *rRecord = malloc(sizeof(RepositoryRecord));
    assert(rRecord != NULL);
    rRecord->record = record;
    TAILQ_INSERT_TAIL(&rQueue, rRecord, entries);
    freeSpace--;
}

unsigned Repository_freeSpace()
{
    return freeSpace;
}

bool Repository_isFree()
{
    return freeSpace == REPOSITORY_SIZE;
}