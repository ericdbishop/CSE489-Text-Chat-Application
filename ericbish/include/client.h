#ifndef CLIENT_H_
#define CLIENT_H_
#include <process.h>

class Client : public Process
{
public:
    char server_ip[INET_ADDRSTRLEN];
    char server_port[6];
    int server_socket;
    int port_listen;
    bool logged_in;
    bool requires_update;
    std::list<char *> blocked_clients;

    Client(char *port);
	int read_inputs();
    void login(char *server_ip, char *server_port);
    void refresh();
    //void send_to_server(char *buffer);
    void send_msg(char *client_ip, char *msg);
    void broadcast(char *msg);
    void block(char *client_ip);
    void unblock(char *client_ip);
    void logout();
    void exit_server();
    void msg_received(char *buffer);
    int require_login(char *cmd);

    void list();
    int call_command(char *command);


    /***************************
          HELPER FUNCTIONS
    ***************************/

    bool isBlocked(char *client_ip);
    int connect_to_host(char *server_ip, char *server_port);
};

#endif