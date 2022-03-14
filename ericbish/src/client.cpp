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

using Process::Process;

  //Client (int port) {
    //port_listen = port;
  //}
};
