#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Server: public Process {
public:
  int port_listen;

void statistics();
void blocked(char client_ip);
void event();

/* I think the following line of code is redundant, according to one guide, the
 * parent constructor of Process will be called automatically */
//using Process::Process;

  Server (int port) : Process(port) {
    // Can this be empty?
  }
};
