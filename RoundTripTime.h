#pragma once

#include "Manager.h"

void RoundTripTime_getWaitTime(RecordTimeout *record, struct timeval *out);
void RoundTripTime_updateRoundTripTime(RecordTimeout *);

void RoundTripTime_init();
void RoundTripTime_free();