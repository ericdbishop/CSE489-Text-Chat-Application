#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Client: public Process {
public:
  int port_listen;

  using Process::Process;

  //Client (int port) {
    //port_listen = port;
  //}
};
