#include <sys/time.h>
#include <aio.h>
#include <stdlib.h>
#include <stdio.h>
#include "Main.h"
#include "Manager.h"

static struct timeval roundTripTime;

void RoundTripTime_getWaitTime(RecordTimeout *record, struct timeval *out)
{
    struct timeval current;
    gettimeofday(&current, NULL);

    timeradd(&roundTripTime, &(record->sendTime), out);

    if (timercmp(&current, out, >))
    {
        out->tv_sec = 0;
        out->tv_usec = 0;
        return;
    }
    timersub(out, &current, out);
    struct timeval zero;
    zero.tv_sec = 0;
    zero.tv_usec = 0;

    timeradd(&zero, out, &current);
    for (int i = RoundTripTimeADD; i > 0; i--)
    {
        timeradd(&current, out, out);
    }
}

void RoundTripTime_updateRoundTripTime(RecordTimeout *tRecord)
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
            printf("Zmiana czasu : %lu %lu\n", roundTripTime.tv_sec, roundTripTime.tv_usec);
        }
    }
}

void RoundTripTime_init()
{
    roundTripTime.tv_sec = RoundTripTimeS;
    roundTripTime.tv_usec = RoundTripTimeUS;
}

void RoundTripTime_free()
{
}