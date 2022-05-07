#pragma once

#include <stdbool.h>

bool Manager_isCompleted();
void Manager_init(unsigned);
void Manager_free();

void Manager_performStep();
void Manager_manageResponse(u_int8_t *);

void Manager_sendRestPakages();