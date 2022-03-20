#ifndef SERVER_H_
#define SERVER_H_
#include"process.h"

struct logged_client:client {
  int num_msg_sent, num_msg_rcv;
  char *status;
  
  logged_client(client to_log){
    num_msg_sent = 0;
    num_msg_rcv = 0;
    strncpy(status, "logged-in", strlen("logged-in"));
    listening_port = to_log.listening_port;
    listening_socket = to_log.listening_socket;
    strncpy(ip, to_log.ip, sizeof(ip));
    strncpy(hostname, to_log.hostname, sizeof(hostname));
  }
};

struct blocked_by:client {
  std::list<client> blocked;
};

class Server: public Process 
{
public:
  std::list<logged_client> logged_clients;
  std::list<blocked_by> block_lists;

  // The process constructor should be called first.
    // Server constructor
  Server (char *port) : Process(port) {}
  void statistics();
  void blocked(char *client_ip);
  void event(char *from_client_ip, char *to_client_ip, char *msg);
  int call_command(char *command);
  bool is_valid_ip(char *client_ip);
  void send_to_server(char *buffer);

  void client_login(char *buffer);
};

#endif