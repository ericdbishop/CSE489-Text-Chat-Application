#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Server: public Process {
public:
  int port_listen;

  void print_port() {
    cse4589_print_and_log("PORT:%d\n", port_listen);
  }

  Server (int port) {
    port_listen = port;
  }
};
