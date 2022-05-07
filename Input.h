#ifndef MY_INPUT
#define MY_INPUT

struct sockaddr_in *Input_getAddr(int argc, char *argv[]);
unsigned Input_getSize(int argc, char *argv[]);
char *Input_getFilePath(int argc, char *argv[]);

#endif