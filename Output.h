#ifndef MY_OUTPUT
#define MY_OUTPUT

#include <netinet/ip.h>

void Output_init(char path[]);
void Output_free();
void Output_write(u_int8_t buffer[],unsigned size);

#endif