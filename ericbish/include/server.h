#ifndef SERVER_H_
#define SERVER_H_
#include"process.h"

struct logged_client:client {
  int num_msg_sent, num_msg_rcv;
  char *status;
};

struct blocked_by:client {
  std::list<client> blocked;
};

class Server: public Process {
  public:
	std::list<logged_client> logged_clients;
    std::list<blocked_by> block_lists;

  // The process constructor should be called first.
    // Server constructor
    Server (int port) : Process(port){

    }
    void statistics();
    void blocked(char *client_ip);
    void event(char *from_client_ip, char *to_client_ip, char *msg, bool broadcast);
}
#endif