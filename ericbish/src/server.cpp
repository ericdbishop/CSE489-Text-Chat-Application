#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Server: public Process {
//Removed the public: because we should not need to redefine variables.

void statistics();
void blocked(char client_ip);
void event();

/* I think the following line of code is redundant, according to one guide, the
 * parent constructor of Process will be called automatically */
//using Process::Process;

  // The process constructor should be called first.
  Server (int port) : Process(port) {
    // Server constructor
  }

};
