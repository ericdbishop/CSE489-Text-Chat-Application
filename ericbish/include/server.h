#ifndef SERVER_H_
#define SERVER_H_
#include"process.h"

/* logged_client struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
struct logged_client:client {
  int num_msg_sent, num_msg_rcv;
  char status[12];
  std::list<char *> buffered_messages;
  
  logged_client(client to_log){
    num_msg_sent = 0;
    num_msg_rcv = 0;
    strcpy(status, "logged-in");
    strcpy(listening_port, to_log.listening_port);
    listening_socket = to_log.listening_socket;
    strcpy(ip, to_log.ip);
    strcpy(hostname, to_log.hostname);
    socket_for_send = to_log.socket_for_send;
  }
	bool operator()(const logged_client one, const logged_client two)
	{
		if (atoi(one.listening_port) > atoi(two.listening_port))
			return false;
		else
			return true;
	}
};

/* each blocked_by structure contains the information of a client and a list of
 * every client they have blocked. This makes it easy to sort the list of
 * blocked clients using the compareClient() comparison. */
struct blocked_by:client {
  std::list<client> blocked;

	bool operator()(const blocked_by one, const blocked_by two)
	{
		if (atoi(one.listening_port) > atoi(two.listening_port))
			return false;
		else
			return true;
	}
};

class Server: public Process 
{
public:
  std::list<logged_client> logged_clients;
  std::list<blocked_by> block_lists;

  // The process constructor should be called first.
    // Server constructor
  Server (char *port);
	int read_inputs();
  void statistics();
  void blocked(char *client_ip);
  void event(char *buffer, int sender);
  int call_command(char *command);
  bool is_valid_ip(char *client_ip);
  void send_to_server(char *buffer);
	void send_connected_clients(int client_socket);

  void client_login(char *buffer, int socket_for_send);
  void client_logout(int sock_fd);

  std::list<client>::iterator find(client *to_find);
  std::list<logged_client>::iterator find(logged_client *to_find);
};

#endif