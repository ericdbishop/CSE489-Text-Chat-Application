#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

class Client {
public:
  int port_listen;

  void print_port() {
    cse4589_print_and_log("PORT:%d\n", port_listen);
  }

  Client (int port) {
    port_listen = port;
  }
}
