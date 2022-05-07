#pragma once

#include <stdbool.h>
#include <sys/queue.h>
#include "Repository.h"
#include <sys/time.h>

typedef struct TRecord
{
    TAILQ_ENTRY(TRecord)
    entries;
    Record *data;
    struct timeval sendTime;
    bool resended;
} RecordTimeout;

void Manager_init(unsigned);
void Manager_free();

void Manager_run();
void Manager_manageResponse(u_int8_t *);

void Manager_sendRestPakages();