//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <cstring>
//#include <fstream>
//#include <unistd.h>
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/process.h"

using namespace std;

/* Create a instance of the client struct for each client that connects. Maintain
 * a list of connected clients for all processes so that they can call list().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */

Process::Process(char *port)
{
	//memset(&self, 0, sizeof(client));
	self.listening_port = port;

	/* Fill in the details for the self Client object */
	makeClient(&self);
}


/* read_inputs() is responsible for calling all other functions and will run so
 * long as the program is running  */
int Process::read_inputs()
{
	struct sockaddr_in client_addr;
	fd_set readfds, master;
	socklen_t caddr_len;
	int fdaccept, fdmax = 0;

	// clear the file descriptor sets
	FD_ZERO(&readfds);
	FD_ZERO(&master);

	FD_SET(STDIN, &master); // add stdin to the file descriptor set

	while (true)
	{
		readfds = master;
		if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			fprintf(stderr, "select error\n");
			// the code in the book makes a call to exit(4)
			// not sure if this is how we should handle the error though
		}
		for (int i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &readfds))
			{ // found a file descriptor

				if (i == STDIN)
					handle_shell(); // HANDLE SHELL COMMANDS

				else if (i == listening_socket) // listener is the servers listening socket fd,
				{ 
					// Accept new connections and add them to master set
					caddr_len = sizeof(client_addr);
					fdaccept = accept(listening_socket, (struct sockaddr *)&client_addr, &caddr_len);
					if (fdaccept < 0)
						perror("Accept failed.");

					printf("\nRemote Host connected!\n");

					/* Add to watched socket list */
					FD_SET(fdaccept, &master);
					if (fdaccept > fdmax)
						fdmax = fdaccept;

					// If this is being called from a server, we should receive the clients information,
					// put that information into a client structure, then add the structure to the
					// connected clients list
					if (program_mode == SERVER) {
						// 1. receive client information in a buffer - might need to wait a second im not sure
						char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
						recv(fdaccept, buffer, BUFFER_SIZE, 0);
						// 2. process the information using receive_connected_client
						// Make sure client is accounted for in logged_clients.
						client_login(buffer);
						// newClient should be updated by receive_connected_client to contain the new client information.
						// 4. send the complete list of connected clients to the client
						send_connected_clients(fdaccept);
					}
					
				}
				else
				{
					// handle data from a client
					/* Initialize buffer to receive response */
					char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
					memset(buffer, '\0', BUFFER_SIZE);

					// See if client disconnected.
					if (recv(i, buffer, BUFFER_SIZE, 0) <= 0)
					{
						close(i);
						printf("Remote Host terminated connection!\n");

						/* Remove from watched list */
						FD_CLR(i, &master);

						// remove from connected clients list TODO

					}
					else
					{
						// Process incoming data from existing clients here ...

						// the flag should be set when the client logs in or refreshes
						if (program_mode == CLIENT /*&& flag */) {
							// clear connected client list
							connected_clients.resize(0);
							
							// do this next part in a loop - we need a flag in the above if statement to
							// signal we are done receiving (or else we will just reset our list)
							// and receive a client continuosly
							while (strcmp(buffer, "DONE") != 0) {
								client *newClient = (client *)malloc(sizeof(client));
								receive_connected_client(buffer, newClient);
							}

							// TODO set flag to signal done receiving
						}
						

						// printf("\nClient sent me: %s\n", buffer);
						// printf("ECHOing it back to the remote host ... ");
						// // I'm pretty sure we don't want to use fdaccept when sending information to the clients
						// if (send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
						// 	printf("Done!\n");
						fflush(stdout);
					}

					free(buffer);
				}
			}
		}
	}
}


/* This function will send the list of connected clients from the server to a
 * client given the client socket number.
 * Returns 1 on success and -1 on failure */
void Process::send_connected_clients(int client_socket)
{
	// for each connected client send their information in a string with the format:
	// listening_port|listening_socket|ip|hostname
  char *buffer;
  int len;
  client currentClient;
  std::list<client>::iterator it;
	for (it=connected_clients.begin(); it != connected_clients.end(); ++it) {
		buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
		client currentClient = (*it);
    buffer = package_client(currentClient); 

		len = strlen(buffer);
		send(client_socket, buffer, len, 0);
	}
	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  strcat(buffer, "DONE\0");
  len = strlen(buffer);
  send(client_socket, buffer, len, 0);
}

// receive connected client as string and process it to be stored in the list of client objects
// "listening_port|listening_socket|ip|hostname"
void Process::receive_connected_client(char *buffer, client *newClient) {
  char *delimiter;
  strncpy(delimiter, "|", 2);
  char *element_str;
  element_str = strtok(buffer, delimiter);

  for (int i = 0; i < 4; i++) {
    // error handling - check if there's less than 4 elements
    if (element_str == NULL) {
      printf("Client::receive_connected_clients error: didn't receive all 4 elements");
    }

    // process the elements and add them to a temporary client
    switch (i)
    {
    case 0:
      newClient->listening_port = element_str;
      break;
    case 1:
      newClient->listening_socket = atoi(element_str);
      break;
    case 2:
      strncpy(newClient->ip, element_str, sizeof(newClient->ip));
      break;
    case 3:
      strncpy(newClient->hostname, element_str, sizeof(newClient->hostname));
      break;
    default:
      break;
    }
    element_str = strtok(buffer, delimiter);
  }
    connected_clients.insert(connected_clients.end(), (*newClient));
}

// Retrieve a command from stdin and after handling the newline character, pass
// it to call_command so that the command can be processed.
void Process::handle_shell(){
	
	// first we read the command from the command line
	char *command = (char *)malloc(sizeof(char) * CMD_SIZE);
	memset(command, '\0', CMD_SIZE);
	if (fgets(command, CMD_SIZE - 1, stdin) == NULL)
	{
		exit(-1);
	}

	string command_string = std::string(command);
	command_string.erase(command_string.size()-1,1);
	char cmd[command_string.size() + 1];
	command_string.copy(cmd, command_string.length() + 1);
	cmd[command_string.length()] = '\0';

	// now we call the corresponding helper functions for each command
	if (call_command(cmd) == -1)
		perror("Command does not exist");
}

/* package_client stores all client information into a char * buffer that
 * contains each piece of information separated by the | character. */
char *Process::package_client(client client_to_package){
  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  char *delimiter, *socket;
  strncpy(delimiter, "|", strlen("|"));

  // buffer structure: listening_port|listening_socket|ip|hostname
  strcat(buffer, client_to_package.listening_port);
  strcat(buffer, delimiter);
  socket = (char *)malloc(sizeof(char) * 6);// 6 because the max port number would be "65535\n"
  sprintf(socket, "%d", client_to_package.listening_socket);
  strcat(buffer, socket); 
  strcat(buffer, delimiter);
  strcat(buffer, client_to_package.ip);
  strcat(buffer, delimiter);
  strcat(buffer, client_to_package.hostname);

  return buffer;
}

/* call_command determines which command function to call based on its input
 * string. Return -1 if the given command does not exist. */
int Process::call_command(char *command)
{
	if (strcmp(command, "AUTHOR") == 0)
		author();
	else if (strcmp(command, "IP") == 0)
		ip();
	else if (strcmp(command, "PORT") == 0)
		port();
	else if (strcmp(command, "LIST") == 0)
		list();
	else
		return -1;

	return 0;
}

/* This function checks the IP's of connected clients and returns true if the
 * given IP is valid, false other wise. This implementation assumes that the
 * given IP is only valid if it belongs to a logged-in client, since it checks
 * connected_clients */
bool Process::is_valid_ip(char *client_ip){
	int acc = 1;
	std::list<client>::iterator i;
	for (i = connected_clients.begin(); i != connected_clients.end(); ++i)
	{
		// retrieve info for the next client in ascending port number order.
		client currentClient = (*i);
		if (client_ip == currentClient.ip)
			return true;
	}
	return false;
}

/* SHELL commands */

/* Both versions of output just combine the shell output and the format string
 * into a small helper function. output_error is for when something goes wrong */
void Process::output(char *cmd, char *format, char *input)
{
	shell_success(cmd);
	cse4589_print_and_log(format, input);
	shell_end(cmd);
}
void Process::output(char *cmd, char *format, int input)
{
	shell_success(cmd);
	cse4589_print_and_log(format, input);
	shell_end(cmd);
}
void Process::output_error(char *cmd)
{
	shell_error(cmd);
	shell_end(cmd);
}

/* Print out author string */
void Process::author()
{
	char *cmd = (char *)"AUTHOR";
	char *format = (char *)"I, %s, have read and understood the course academic integrity policy.\n";
	char *name = (char *)"ericbish";

	output(cmd, format, name);
}

/* ip() print out the ip assosciated with this process. */
void Process::ip()
{	// uses code from section 6.3 of Beej's Guide to Network Programming
	char *cmd = (char *)"IP";
	char *format = (char *)"IP:%s\n";
	// Print output
	output(cmd, format, self.ip);
}

/* port() will print out the port assosciated with this process (client or
 * server) */
void Process::port()
{
	char *cmd = (char *)"PORT";
	char *format = (char *)"PORT:%d\n";
	output(cmd, format, atoi(self.listening_port));
}

/* list() should print out each client in the list of clients connected_clients,
 * according to the format string */
void Process::list()
{
	int list_id, port_listen;
	char *hname, *ip_addr;

	char *cmd = (char *)"LIST";
	shell_success(cmd);
	int acc = 1;
    std::list<client>::iterator i;
	for (i = connected_clients.begin(); i != connected_clients.end(); ++i)
	{
		// retrieve info for the next client in ascending port number order.
		client currentClient = (*i);
		list_id = acc;
		port_listen = atoi(currentClient.listening_port);
		hname = currentClient.hostname;
		ip_addr = currentClient.ip;
		acc++;
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hname, ip_addr, port_listen);
	}
	shell_end(cmd);
}

// This will not be called unless the process is a server.
void Process::client_login(char *buffer){}

// End Process class

/* This helper function creates the socket we listen for new connections on,
	* it should be called during initialization of the Server
	*/
void create_listener(client *newClient) {
	int head_socket, selret, sock_index, fdaccept = 0, caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, newClient->listening_port, &hints, &res) != 0)
	{
		perror("getaddrinfo failed");
	}

	/* Socket */
	newClient->listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (newClient->listening_socket < 0)
	{
		perror("Cannot create socket");
	}

	/* Bind */
	if (bind(newClient->listening_socket, res->ai_addr, res->ai_addrlen) < 0)
	{
		perror("Bind failed");
	}

	freeaddrinfo(res);

	/* Listen */
	if (listen(newClient->listening_socket, BACKLOG) < 0)
	{
		perror("Unable to listen on port");
	}
}

/* Return 1 on success, -1 otherwise */
int makeClient(client *newClient)
{

	char *cmd = (char *)"IP";
	char ipstr[INET_ADDRSTRLEN];
	struct addrinfo hints, *res;
	int sockfd, status;

	// load up adress structs with getaddrinfo()
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo("8.8.8.8", "53", &hints, &res)) != 0)
	{
		// Do we need the same error checking here if it is outside of the IP command?
		fprintf(stderr, "IP: getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	// make a socket
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		fprintf(stderr, "IP: socket error\n");
		return -1;
	}

	// connect
	if ((connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
	{
		fprintf(stderr, "IP: connect error\n");
		return -1;
	}

	// get my IP Address
	struct sockaddr_in myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	socklen_t len = sizeof(myaddr);
	if ((getsockname(sockfd, (struct sockaddr *)&myaddr, &len)) == -1)
	{
		fprintf(stderr, "IP: getsockname error\n");
		return -1;
	}

	if ((inet_ntop(AF_INET, &(myaddr.sin_addr), newClient->ip, sizeof(ipstr))) == NULL)
	{
		fprintf(stderr, "IP: inet_ntop error\n");
		return -1;
	}

	// close UDP socket
	if ((close(sockfd)) == -1)
	{
		fprintf(stderr, "IP: close error\n:");
		return -1;
	}

	//newClient->ip = ipstr;
	// myaddr is the whole sockaddr_in struct, we are getting the size of just
	// the sin_addr here.
	hostent *host = gethostbyaddr((char *)&myaddr.sin_addr.s_addr, sizeof(struct in_addr), AF_INET);
	// strncpy(ret->im_host, hent->h_name, sizeof(ret->im_host) - 1);
	std::strncpy(newClient->hostname, host->h_name, sizeof(newClient->hostname) - 1);

	// create the listening socket for the specified port
	create_listener(newClient);
	return 1;
};

/* Helper functions for SHELL output */
void shell_success(char *command_str)
{
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
}

void shell_end(char *command_str)
{
	cse4589_print_and_log("[%s:END]\n", command_str);
}

void shell_error(char *command_str)
{
	cse4589_print_and_log("[%s:ERROR]", command_str);
	shell_end(command_str);
}