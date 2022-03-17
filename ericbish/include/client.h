#ifndef CLIENT_H_
#define CLIENT_H_
#include <list>
#include <process.h>

using namespace std;

class Client : public Process
{
public:
    char *server_ip;
    char *server_port;
    int server_socket;
    int port_listen;
    bool logged_in = false;
    std::list<client> blocked_clients;
    void login(char *server_ip, char *server_port);
    void refresh();
    void send(char *client_ip, char *msg);
    void broadcast(char *msg);
    void block(char *client_ip);
    void unblock(char *client_ip);
    void logout();
    void exit();
    void msg_received(char *client_ip, char *msg);
    bool isBlocked(char *client_ip);
    Client(int port);
    int call_command(char *command);
    void require_login(char *cmd);
    void list();
    void login(char *server_ip, char *server_port);
    void refresh();
    void send(char *client_ip, char *msg);
    void broadcast(char *msg);
    void block(char *client_ip);
    void unblock(char *client_ip);
    void logout();
    void exit();
    void msg_received(char *client_ip, char *msg);

    /***************************
          HELPER FUNCTIONS
    ***************************/

    bool isBlocked(char *client_ip);
    int connect_to_host(char *server_ip, char *server_port);
};

#endif