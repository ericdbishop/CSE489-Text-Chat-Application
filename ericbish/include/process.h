#ifndef PROCESS_H_
#define PROCESS_H_
#include <list>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <netdb.h>
#include <string>

using namespace std;

#define STDIN 0
#define CMD_SIZE 100
#define BACKLOG 5
#define BUFFER_SIZE 256

#define SERVER true
#define CLIENT false

struct client
{
	char *listening_port;
	int listening_socket;
	char ip[INET_ADDRSTRLEN];
	char hostname[128];
	/* compareClient provides a sorting function for the connected_clients linked list */
	bool operator()(const client one, const client two)
	{
		if (atoi(one.listening_port) > atoi(two.listening_port))
			return false;
		else
			return true;
	}
};

void shell_success(char *command_str);
void shell_end(char *command_str);
void shell_error(char *command_str);
void create_listener(client *newClient);
int makeClient(client *newClient);

class Process{
    public:
	struct client self;
	bool program_mode;
	// listening_socket is the socket fd.
	int listening_socket;
	std::list<client> connected_clients;

	Process(char *port);
	int read_inputs();
	void handle_shell();
	int call_command(char *command);
	bool is_valid_ip(char *client_ip);
	void output(char *cmd, char *format, char *input);
	void output(char *cmd, char *format, int input);
    void output_error(char *cmd);
	void author();
	void ip();
    void port();
    void list();
	void send_connected_clients(int client_socket);
	void receive_connected_client(char *buffer, client *newClient);
	
	char *package_client(client client_to_package); 
	char *package(std::list<char *> segments);
	
	void client_login(char *buffer);
};

#endif