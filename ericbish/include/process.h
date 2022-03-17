
#ifndef PROCESS_H_
#define PROCESS_H_
#include <list>

using namespace std;

struct client
{
	int listening_port;
	int listening_socket;
	char *ip;
	char hostname[128];
	/* compareClient provides a sorting function for the connected_clients linked list */
	bool operator()(const client one, const client two)
	{
		if (one.listening_port > two.listening_port)
			return false;
		else
			return true;
	}
};

void shell_success(char *command_str);
void shell_end(char *command_str);
void shell_error(char *command_str);
int makeClient(client *newClient);

class Process{
    public:
	struct client *self;
	int listening_socket;
	std::list<client> connected_clients;

	Process(int port);
	int read_inputs();
	int call_command(char *command);
	bool is_valid_ip(char *client_ip);
	void output(char *cmd, char *format, char *input);
	void output(char *cmd, char *format, int input);
	void author();
	void ip();
}

#endif