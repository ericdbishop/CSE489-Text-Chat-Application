#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

class Client: public Process {
public:
  int port_listen;
  bool logged_in = false;
  std::list<client> blocked;

  void login(char *server_ip, char *server_port);
  void refresh();
  void send(char *client_ip, char *msg);
  void broadcast(char *msg);
  void block(char *client_ip);
  void unblock(char *client_ip);
  void logout();
  void exit();
  // msgReceived will handle incoming messages and print/log them
  void msg_received(char *client_ip, char *msg);
  bool isBlocked(char *client_ip)

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

  void require_login(char *cmd){
    if (!logged_in){
      shell_error(cmd);
    }
  }
  /* Invalid IP address. Valid IP address which does not exist in the local copy
   * of the list of logged-in clients (This list may be outdated. Do not update
   * it as a result of this check). Client with IP address: <client-ip> is already blocked. */
    
  /* A client should not accept any other command, except LOGIN, EXIT, IP, PORT,
   * and AUTHOR, or receive packets, unless it is successfully logged in to the
   * server. */

  /* Overload list() command to ensure that the client is logged in, and then
   * call parent function. */
  void list() {
    char *cmd = "LIST";
    require_login(cmd);
    Process::list();
  }

  void login(char server_ip, char server_port){
    char *cmd = "LOGIN";
    
  }

  /* Retrieve an updated list of loggin in clients from the server and use it to update connected_clients */
  void refresh(){
    char *cmd = "REFRESH";
    require_login(cmd);
  }
  
  void send(char client_ip, char msg){
    char *cmd = "SEND";
    require_login(cmd);
  }
  
  void broadcast(char msg){
    char *cmd = "BROADCAST";
    require_login(cmd);
  
  }
  
  void block(char client_ip){
    char *cmd = "BLOCK";
    require_login(cmd);
  
  }
  
  void unblock(char client_ip){
    char *cmd = "UNBLOCK";
    require_login(cmd);
  
  }
  
  void logout(){
    char *cmd = "LOGOUT";
    require_login(cmd);
  
  }
  
  void exit(){
    char *cmd = "EXIT";
  
  }
  
  // msgReceived will handle incoming messages and print/log them
  void msg_received(char *client_ip, char *msg){
	  char *format = "msg from:%s\n[msg]:%s\n";
    char *cmd = "RECEIVED";

    shell_success(cmd);
    cse4589_print_and_log(format, client_ip, msg);
    shell_end(cmd);
  }

  /***************************
        HELPER FUNCTIONS
  ***************************/

  bool isBlocked(char *client_ip){

  }

};
