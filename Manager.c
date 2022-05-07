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
#include <sys/time.h>
#include <aio.h>
#include "Main.h"
#include "Socket.h"
#include "Repository.h"

typedef struct TRecord
{
    TAILQ_ENTRY(TRecord)
    entries;
    Record *data;
    struct timeval sendTime;
    bool resended;
} RecordTimeout;

TAILQ_HEAD(TaskQueue, TRecord);

static struct TaskQueue tQueue;

static unsigned size = 0;
static unsigned start = 0;
static struct timeval roundTripTime;

bool Manager_isCompleted()
{
    return size == start && Repository_isFree();
}

/* INIT */

void Manager_init(unsigned _size)
{
    assert(start == 0);
    size = _size;
    TAILQ_INIT(&tQueue);
    roundTripTime.tv_sec = RoundTripTimeS;
    roundTripTime.tv_usec = RoundTripTimeUS;
}

void Manager_free()
{
    size = 0;
    start = 0;
    while (!TAILQ_EMPTY(&tQueue))
        TAILQ_REMOVE(&tQueue, TAILQ_FIRST(&tQueue), entries);
}

/* Helpers */

static unsigned _lenInNextPackage()
{
    return size - start > PACKAGE_MAX_LEN ? PACKAGE_MAX_LEN : size - start;
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

/* Task */

static void _send(RecordTimeout *record)
{
    gettimeofday(&(record->sendTime), NULL);
    Socket_send(record->data);
}

static void _addTask(unsigned start, unsigned lenLeft)
{
    Record *record = _createRecordHeader(start, lenLeft);

    RecordTimeout *tRecord = malloc(sizeof(RecordTimeout));
    tRecord->resended = false;
    tRecord->data = record;
    TAILQ_INSERT_TAIL(&tQueue, tRecord, entries);
  //  fprintf(stderr, "Dodanie tasku %u %u\n", record->start, record->size);
    _send(tRecord);
    Repository_addRequest(record);
}

static void _resendFirstInQueue()
{
    RecordTimeout *first = TAILQ_FIRST(&tQueue);
    first->resended = true;
    TAILQ_REMOVE(&tQueue, first, entries);
    TAILQ_INSERT_TAIL(&tQueue, first, entries);
 //   fprintf(stderr, "Ponowne wysłanie %u %u\n", first->data->start, first->data->size);
    _send(first);
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

/* HL */

static void _calculate_time(RecordTimeout *record, struct timeval *out)
{
    struct timeval current;
    gettimeofday(&current, NULL);

    timeradd(&roundTripTime, &(record->sendTime), out);

    if (timercmp(&current, out, >))
    {
        out->tv_sec = 0;
        out->tv_usec = 0;
     //   fprintf(stderr, "Obliczany czas czekania: %u %u\n", out->tv_sec, out->tv_usec);
        return;
    }
    timersub(out, &current, out);
  //  fprintf(stderr, "RAW Obliczany czas czekania: %u %u\n", out->tv_sec, out->tv_usec);
    for (int i = RoundTripTimeADD; i > 0; i--)
    {
        timeradd(out, out, out);
    }
  //  fprintf(stderr, "Obliczany czas czekania: %u %u\n", out->tv_sec, out->tv_usec);
}

static void _updateRoundTripTime(RecordTimeout *tRecord)
{
    if (!tRecord->resended)
    {
        struct timeval newRoundTrip, current;
        gettimeofday(&current, NULL);
        timersub(&current, &(tRecord->sendTime), &newRoundTrip);
        if (timercmp(&roundTripTime, &newRoundTrip, >))
        {
            roundTripTime.tv_sec = newRoundTrip.tv_sec;
            roundTripTime.tv_usec = newRoundTrip.tv_usec;
           // fprintf(stderr, "Zmiana roundTrpiu %u %u\n", roundTripTime.tv_sec, roundTripTime.tv_usec);
        }
    }
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

void Manager_manageResponse(u_int8_t *buffor)
{
    Record *record = _bufforToRecord(buffor);
    RecordTimeout *tRecord;
    TAILQ_FOREACH(tRecord, &tQueue, entries)
    if (tRecord->data->start == record->start && tRecord->data->size == record->size)
    {
        //fprintf(stderr, "Dotarła odpowiedz %u %u\n", record->start, record->size);
        TAILQ_REMOVE(&tQueue, tRecord, entries);
        Repository_addResponse(record);
        _updateRoundTripTime(tRecord);
        free(tRecord);
        return;
    }

    free(record->buffer);
    free(record);
}

void Manager_performStep()
{
    Manager_sendRestPakages();
    RecordTimeout *first;
    while (!TAILQ_EMPTY(&tQueue))
    {
        first = TAILQ_FIRST(&tQueue);
        struct timeval wait;
        _calculate_time(first, &wait);
        Socket_receive(wait);
        if (first == TAILQ_FIRST(&tQueue))
            _resendFirstInQueue();
    }
}
