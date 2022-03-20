#ifndef SERVER_H_
#define SERVER_H_
#include"process.h"

/* logged_client struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
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

/* each blocked_by structure contains the information of a client and a list of
 * every client they have blocked. This makes it easy to sort the list of
 * blocked clients using the compareClient() comparison. */
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
	int read_inputs();
  void statistics();
  void blocked(char *client_ip);
  void event(char *from_client_ip, char *to_client_ip, char *msg);
  int call_command(char *command);
  bool is_valid_ip(char *client_ip);
  void send_to_server(char *buffer);

  void client_login(char *buffer);
  void client_logout(int sock_fd);

  std::list<client>::iterator find(client *to_find);
  std::list<logged_client>::iterator find(logged_client *to_find);
};

#endif