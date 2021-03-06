#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "Input.h"
#include "Main.h"
#include "Output.h"
#include "Socket.h"
#include "Manager.h"
#include "Repository.h"

int main(int argc, char *argv[])
{
  Output_init(Input_getFilePath(argc, argv));
  Repository_init();
  Manager_init(Input_getSize(argc, argv));
  Socket_init(Input_getAddr(argc, argv));

  Manager_run();

  Socket_free();
  Manager_free();
  Repository_free();
  Output_free();

  return EXIT_SUCCESS;
}