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
#include "Socket.h"
#include "Repository.h"
#include "Manager.h"
#include "RoundTripTime.h"

TAILQ_HEAD(TaskQueue, TRecord);
static struct TaskQueue tQueue;

static unsigned size = 0;
static unsigned start = 0;

/* Private */

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
    assert(sscanf((char *)buffor, "DATA %u %u\n", &start, &size) == 2);
    while (buffor[offset++] != '\n' && offset < 100)
        ;
    assert(offset < 100);

    Record *record = _createRecordHeader(start, size);
    record->buffer = malloc(sizeof(u_int8_t) * size);
    assert(record->buffer != NULL);
    for (unsigned dataIntex = 0; dataIntex < size; dataIntex++)
    {
        record->buffer[dataIntex] = buffor[dataIntex + offset];
    }

    return record;
}

static unsigned _lenInNextPackage()
{
    return size - start > PACKAGE_MAX_LEN ? PACKAGE_MAX_LEN : size - start;
}

static void _send(RecordTimeout *record)
{
    gettimeofday(&(record->sendTime), NULL);
    Socket_send(record->data);
}

static void _addTask(unsigned start, unsigned lenLeft)
{
    Record *record = _createRecordHeader(start, lenLeft);

    RecordTimeout *tRecord = malloc(sizeof(RecordTimeout));
    assert(tRecord != NULL);
    tRecord->resended = false;
    tRecord->data = record;
    TAILQ_INSERT_TAIL(&tQueue, tRecord, entries);
    _send(tRecord);
    Repository_addRequest(record);
}

static void _resendFirstInQueue()
{
    RecordTimeout *first = TAILQ_FIRST(&tQueue);
    first->resended = true;
    TAILQ_REMOVE(&tQueue, first, entries);
    TAILQ_INSERT_TAIL(&tQueue, first, entries);
    _send(first);
}

/* Public */

void Manager_manageResponse(u_int8_t *buffor)
{
    Record *record = _bufforToRecord(buffor);
    RecordTimeout *tRecord;
    TAILQ_FOREACH(tRecord, &tQueue, entries)
    if (tRecord->data->start == record->start && tRecord->data->size == record->size)
    {
        TAILQ_REMOVE(&tQueue, tRecord, entries);
        Repository_addResponse(record);
        RoundTripTime_updateRoundTripTime(tRecord);
        free(tRecord);
        return;
    }

    free(record->buffer);
    free(record);
}

void Manager_sendRestPakages()
{
    unsigned lenLeft, freeSpace = Repository_freeSpace();
    while (freeSpace-- > 0 && (lenLeft = _lenInNextPackage()) > 0)
    {
        _addTask(start, lenLeft);
        start += lenLeft;
    }
}

void Manager_run()
{
    Manager_sendRestPakages();

    RecordTimeout *first;
    while (!TAILQ_EMPTY(&tQueue))
    {
        first = TAILQ_FIRST(&tQueue);
        struct timeval wait;
        RoundTripTime_getWaitTime(first, &wait);

        Socket_receive(wait);

        if (first == TAILQ_FIRST(&tQueue))
            _resendFirstInQueue();
    }
}

/* INIT */

void Manager_init(unsigned _size)
{
    assert(start == 0);
    size = _size;
    TAILQ_INIT(&tQueue);

    RoundTripTime_init();
}

void Manager_free()
{
    size = 0;
    start = 0;
    while (!TAILQ_EMPTY(&tQueue))
        TAILQ_REMOVE(&tQueue, TAILQ_FIRST(&tQueue), entries);

    RoundTripTime_free();
}
