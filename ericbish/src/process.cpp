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

// receive connected client as string and process it to be stored in the list of client objects
// "listening_port|listening_socket|ip|hostname"
void Process::receive_connected_client(char *buffer) {
  char *delimiter = "|";
  char *element_str;
  element_str = strtok(buffer, delimiter);
  struct client temp; 
  for (int i = 0; i < 4; i++) {
    // error handling - check if there's less than 4 elements
    if (element_str == NULL) {
      printf("Client::receive_connected_clients error: didn't receive all 4 elements");
    }

    // process the elements and add them to a temporary client
    switch (i)
    {
    case 0:
      temp.listening_port = element_str;
      break;
    case 1:
      temp.listening_socket = atoi(element_str);
      break;
    case 2:
      strncpy(temp.ip, element_str, sizeof(temp.ip));
      break;
    case 3:
      strncpy(temp.hostname, element_str, sizeof(temp.hostname));
      break;
    default:
      break;
    }
    element_str = strtok(buffer, delimiter);
  }
    connected_clients.insert(connected_clients.end(), temp);
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
				{
					// HANDLE SHELL COMMANDS

					// first we read the command from the command line
					char *command = (char *)malloc(sizeof(char) * CMD_SIZE);
					memset(command, '\0', CMD_SIZE);
					if (fgets(command, CMD_SIZE - 1, stdin) == NULL)
					{
						exit(-1);
					}

					string command_string = std::string(command);
					command_string.erase(command_string.size()-1,1);
					//command_string.find('\n')
					char cmd[command_string.size() + 1];
					command_string.copy(cmd, command_string.length() + 1);
					cmd[command_string.length()] = '\0';

					// now we call the corresponding helper functions for each command
					call_command(cmd);
				}
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

					// Now if we are instantiated as the server we should receive the clients information,
					// put that information into a client structure, then add the structure to the
					// connected clients list
					if (program_mode == SERVER) {
						// 1. receive client information in a buffer - might need to wait a second im not sure
						char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
						recv(fdaccept, buffer, BUFFER_SIZE, 0);
						
						// 2. proccess the information using receive_connected_client
						receive_connected_client(buffer);

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


						if (program_mode == CLIENT /*&& the client logged in/refreshed*/) {
							// clear connected client list
							connected_clients.resize(0);
							
							// do this next part in a loop - we need a flag in the above if statement to
							// signal we are done receiving (or else we will just reset our list)
							// and receive a client continuosly
							receive_connected_client(buffer);
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

char *Process::package_client(client client_to_package){
	char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  	char *delimiter = "|";

	strcat(buffer, client_to_package.listening_port);
	strcat(buffer, delimiter);
	char *socket = (char *)malloc(sizeof(char) * 6);// 6 because the max port number would be "65535\n"
	sprintf(socket, "%d", client_to_package.listening_socket);
	strcat(buffer, socket); 
	strcat(buffer, delimiter);
	strcat(buffer, client_to_package.ip);
	strcat(buffer, delimiter);
	strcat(buffer, client_to_package.hostname);

	return buffer;
}
// call_command determines which function to call based on its input string
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
 * given IP is only valid if it belongs to a logged-in client.  */
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

void Process::author()
{
	char *cmd = (char *)"AUTHOR";
	char *format = (char *)"I, %s, have read and understood the course academic integrity policy.\n";
	char *name = (char *)"ericbish";

	output(cmd, format, name);
}

void Process::ip()
{	// uses code from section 6.3 of Beej's Guide to Network Programming
	/* We still need to have some error handling in here, from how the PA1
	 * description is written. So we could maybe have a separate helper
	 * function that we call once when a process begins, to gather its
	 * hostname and external ip and whatnot, and then everytime the IP
	 * command is given we can call it again. The helper function will detect
	 * errors and return -1 if something is wrong. */

	char *cmd = (char *)"IP";
	char *format = (char *)"IP:%s\n";
	/* Will this prodcue issues if self has already been defined? */
	//int result = makeClient(self);

	// if (result == -1)
	// {
	// 	shell_error(cmd);
	// 	return;
	// }
	
	// Print output
	output(cmd, format, self.ip);
}

void Process::port()
{
	char *cmd = (char *)"PORT";
	char *format = (char *)"PORT:%d\n";
	output(cmd, format, atoi(self.listening_port));
}

/* list() should  */
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

; // End Process class

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