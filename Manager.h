#ifndef MY_MANAGER
#define MY_MANAGER

#include <stdbool.h>

bool Manager_isCompleted();
void Manager_init(unsigned _size);
void Manager_free();
void Manager_performStep();

#endif