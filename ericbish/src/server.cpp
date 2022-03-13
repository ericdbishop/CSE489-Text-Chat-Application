#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Server: public Process {
public:
  int port_listen;

  using Process::Process;

  //Server (int port) {
    //port_listen = port;
  //}
};
