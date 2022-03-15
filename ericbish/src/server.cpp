#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

/* loggedClient struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
struct loggedClient: Client {
	int num_msg_sent, num_msg_rcv;
};

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

  /* statistics displays a numbered list of clients who are or have previously
   * logged in to the server, who have not used the EXIT command, 
   * with statistics such as their number of sent and received
   * messages, and whether they are logged in or out. */
  void statistics(){

  }
};
