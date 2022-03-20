#ifndef CLIENT_H_
#define CLIENT_H_
#include <process.h>

class Client : public Process
{
public:
    char *server_ip;
    char *server_port;
    int server_socket;
    int port_listen;
    bool logged_in;
    std::list<client> blocked_clients;

    Client(char *port);
    void login(char *server_ip, char *server_port);
    void refresh();
    char *package_self();
    void send_to_server(char *buffer);
    void send(char *client_ip, char *msg);
    void broadcast(char *msg);
    void block(char *client_ip);
    void unblock(char *client_ip);
    void logout();
    void exit();
    void msg_received(char *client_ip, char *msg);
    void require_login(char *cmd);

    void list();
    int call_command(char *command);


    /***************************
          HELPER FUNCTIONS
    ***************************/

    bool isBlocked(char *client_ip);
    int connect_to_host(char *server_ip, char *server_port);
};

#endif