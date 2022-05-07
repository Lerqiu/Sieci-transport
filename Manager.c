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
#include "Socket.h"
#include "Repository.h"

static unsigned size = 0;
static unsigned start = 0;

bool Manager_isCompleted()
{
    return size == start && Repository_isFree();
}

void Manager_init(unsigned _size)
{
    assert(start == 0);
    size = _size;
}

void Manager_free()
{
    size = 0;
    start = 0;
}

static unsigned _lenInNextPackage()
{
    return size - start > PACKAGE_MAX_LEN ? PACKAGE_MAX_LEN : size - start;
}

static void _resendRecordIfNeed(Record *record)
{
    if(record->buffer == NULL)
        Socket_resend(record);
}

void Manager_performStep()
{
    unsigned lenLeft, freeSpace = Repository_freeSpace();
    while (freeSpace-- > 0 && (lenLeft = _lenInNextPackage()) > 0)
    {
        Socket_send(start, lenLeft);
        start += lenLeft;
    }
    // Get min time
    Repository_forEach(_resendRecordIfNeed);

    Socket_receive(MAXTIMEOUT);
    Repository_flush();
}