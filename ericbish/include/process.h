
#ifndef PROCESS_H_
#define PROCESS_H_

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

#endif