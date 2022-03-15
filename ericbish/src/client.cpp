#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Client: public Process {
public:
  int port_listen;

void login(char server_ip, char server_port);
void refresh();
void send(char client_ip, char msg);
void broadcast(char msg);
void block(char client_ip);
void unblock(char client_ip);
void logout();
void exit();
// msgReceived will handle incoming messages and print/log them
void msgReceived();

/* I think the following line of code is redundant, according to one guide, the
 * parent constructor of Process will be called automatically */
//using Process::Process;

  Client (int port) : Process(port) {
    // Add self Client object to list of connected clients.
	  connected_clients.insert(connected_clients.begin(), *self);

	  // Sorting isn't neccesary here if self is the only client in the list
	  // but for future reference this is how we sort:
	  // connected_clients.sort(compareClient());
  }
};
