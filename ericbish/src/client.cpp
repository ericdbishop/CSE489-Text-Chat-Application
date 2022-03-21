#include "../include/global.h"
#include "../include/logger.h"
#include "../include/client.h"

Client::Client (char *port) : Process(port) {
  // Add self Client object to list of connected clients.
  logged_in = false;
  requires_update = true;

 // but for future reference this is how we sort:
 // connected_clients.sort(compareClient());
}

/* read_inputs() is responsible for calling all other functions and will run so
 * long as the program is running. Much of this is taken from/based off of the bgnet guide */
int Client::read_inputs(){

	struct sockaddr_in server_addr;
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

				if (i == STDIN) {
          // Handle shell input
	        // now we call the corresponding helper functions for each command
	        if (call_command(handle_shell()) == -1)
		        perror("Command does not exist");
        }
				else if (i == listening_socket) // listening socket fd,
				{ 
					// Accept new connections and add them to master set
					caddr_len = sizeof(server_addr);
					fdaccept = accept(listening_socket, (struct sockaddr *)&server_addr, &caddr_len);
					if (fdaccept < 0)
						perror("Accept failed.");

					printf("\nRemote Host connected!\n");

					/* Add to watched socket list */
					FD_SET(fdaccept, &master);
					if (fdaccept > fdmax)
						fdmax = fdaccept;
				}
				else
				{
					// handle incoming data from server
					/* Initialize buffer to receive response */
					char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
					//Why set this to all terminating bytes?
					//memset(buffer, '\0', BUFFER_SIZE);

					int nbytes;
					if (nbytes = recv(i, buffer, BUFFER_SIZE, 0) <= 0)
					{ // got error or connection closed by client
					 	if (nbytes == 0)
						  printf("socket %d hung up", i);
						else
						  perror("recv");

						close(i);
						printf("Remote Host terminated connection!\n");

						/* Remove from watched list */
						FD_CLR(i, &master);
					}
					else
					{
						// Process incoming data from existing clients here ...

						// the flag should be set when the client logs in or refreshes
						if (requires_update) {
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

// call_command will add a \0 to the end of each input string.
int Client::call_command(char *command){
  if (Process::call_command(command) == 0) return 0;

  string cmd_and_arguments = std::string(command);
  size_t cmd_length = cmd_and_arguments.length();
  string cmd, arguments; 

  if (strcmp(command, "REFRESH") == 0)
    refresh();
  else if (strcmp(command, "LOGOUT") == 0)
    logout();
  else if (strcmp(command, "EXIT") == 0)
    exit_server();
  
  if (cmd_and_arguments.length() < 10){
    perror("Command is too short to have valid arguments");
  }

  if (cmd_and_arguments.find("SEND") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,4);
    // Arguments here will be both arguments separated by a space.
    string arguments = cmd_and_arguments.substr(5);
    string ip = arguments.substr(0,arguments.find(" "));
    string message = arguments.substr(arguments.find(" ")+1);

    char client_ip[ip.size() + 1];
    char msg[message.size() + 1];
    
    ip.copy(client_ip, ip.length() + 1);
    client_ip[ip.length()] = '\0';

    message.copy(msg, message.length() + 1);
    msg[message.length()] = '\0';

    //Check if the IP is valid
    if (!is_valid_ip(client_ip)) {
      // error
      perror("Invalid IP");
    }

    send(client_ip, msg);
  }
  else if (cmd_and_arguments.find("LOGIN") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,5);
    string arguments = cmd_and_arguments.substr(6);
    string ip = arguments.substr(0,arguments.find(" "));
    string port = arguments.substr(arguments.find(" ")+1);
    // Arguments here will be both arguments separated by a space.
    // Split arguments so we can separate the ip and port
    char s_ip[ip.size() + 1];
    char s_port[port.size() + 1];

    ip.copy(s_ip, ip.length() + 1);
    s_ip[ip.length()] = '\0';

    port.copy(s_port, port.length() + 1);
    s_port[port.length()] = '\0';

    // These will be our server ip and port
    strcpy(server_ip, s_ip);
    strcpy(server_port, s_port);

    printf(server_ip);
    printf(server_port);
    login(server_ip, server_port);
  }
  else if (cmd_and_arguments.find("BLOCK") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,5);
    string arguments = cmd_and_arguments.substr(6);
    char client_ip[arguments.size() + 1];

    arguments.copy(client_ip, arguments.length() + 1);
    client_ip[arguments.length()] = '\0';

    //Check if the IP is valid
    if (!is_valid_ip(client_ip)) {
      // error
      perror("Invalid IP");
    }

    block(client_ip);
  }
  else if (cmd_and_arguments.find("UNBLOCK") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,7);
    string arguments = cmd_and_arguments.substr(8);
    char client_ip[arguments.size() + 1];

    arguments.copy(client_ip, arguments.length() + 1);
    client_ip[arguments.length()] = '\0';

    //Check if the IP is valid
    if (!is_valid_ip(client_ip)) {
      // error
      perror("Invalid IP");
    }

    unblock(client_ip);
  }
  else if (cmd_and_arguments.find("BROADCAST") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,9);
    string arguments = cmd_and_arguments.substr(10);
    char msg[arguments.size() + 1];

    arguments.copy(msg, arguments.length() + 1);
    msg[arguments.length()] = '\0';

    broadcast(msg);
  }
  else return -1;

 return 0;
}

// helper function for LOGIN command - taken from client.c in example code.
int Client::connect_to_host(char *server_ip, char* server_port)
{
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/* Fill up address structures */	
	if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0)
		perror("Failed to create socket");
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");
	
  /* Wait and then send information, as the server should be awaiting our information at this point*/
  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	buffer = package_client(self);

  usleep(10000);

  send_to_server(buffer);

	freeaddrinfo(res);

	return fdsocket;
}

void Client::send_to_server(char *buffer){

}

void Client::require_login(char *cmd){
  if (!logged_in){
    shell_error(cmd);
  }
}
  
/* A client should not accept any other command, except LOGIN, EXIT, IP, PORT,
 * and AUTHOR, or receive packets, unless it is successfully logged in to the
 * server. */

/* Overload list() command to ensure that the client is logged in, and then
 * call parent function. */
void Client::list() {
  char *cmd = (char *)"LIST";
  require_login(cmd);
  Process::list();
}

void Client::login(char *server_ip, char *server_port){
  char *cmd = (char *)"LOGIN";
  server_socket = connect_to_host(server_ip, server_port);
  if (server_socket < 0) {
    output_error(cmd);
    exit(-1);
  }

  // now we can use server_socket to send and receive data

  // on login the server should send the client the list of currently connected clients

  // Maybe server should handle changing connected clients and the client should just request a new list of clients.
  logged_in = true;
  shell_success(cmd);
  shell_end(cmd);
}

void Client::logout(){
  char *cmd = (char *)"LOGOUT";
  require_login(cmd);

  close(server_socket);

  // They should not be able to view connected clients and logged_in should be changed to false
  // connected_clients.remove(self);
  connected_clients.resize(0);
  logged_in = false;
  shell_success(cmd);
  shell_end(cmd);
}

/* Retrieve an updated list of loggin in clients from the server and use it to update connected_clients */
void Client::refresh(){
  char *cmd = (char *)"REFRESH";
  require_login(cmd);
  shell_success(cmd);
  shell_end(cmd);
}

void Client::send(char *client_ip, char *msg){
/* Exceptions:

 * Invalid IP address. 

 * Valid IP address which does not exist in the local copy
 * of the list of logged-in clients (This list may be outdated. Do not update
 * it as a result of this check). */
  char *cmd = (char *)"SEND";
  require_login(cmd);
  shell_success(cmd);
  shell_end(cmd);
}

void Client::broadcast(char *msg){
  char *cmd = (char *)"BROADCAST";
  require_login(cmd);
  shell_success(cmd);
  shell_end(cmd);

}

void Client::block(char *client_ip){
/* Exceptions:
 * 
 * Invalid IP address. 
 * 
 * Valid IP address which does not exist in the local copy
 * of the list of logged-in clients (This list may be outdated. Do not update
 * it as a result of this check). 
 * 
 * Client with IP address: <client-ip> is already blocked.   */

  char *cmd = (char *)"BLOCK";
  require_login(cmd);

  /* */
  if (isBlocked(client_ip)){
    shell_error(cmd);
  }

  /* TO DO: NOTIFY SERVER */


  shell_success(cmd);
  shell_end(cmd);
}

void Client::unblock(char *client_ip){
/* Exceptions:
 * 
 * Invalid IP address. 
 * 
 * Valid IP address which does not exist in the local copy
 * of the list of logged-in clients (This list may be outdated. Do not update
 * it as a result of this check). 
 * 
 * Client with IP address: <client-ip> is not blocked.   */

  char *cmd = (char *)"UNBLOCK";
  require_login(cmd);

  /* TO DO: NOTIFY SERVER */


  shell_success(cmd);
  shell_end(cmd);
}


void Client::exit_server(){
  char *cmd = (char *)"EXIT";

  shell_success(cmd);
  shell_end(cmd);
}

// msgReceived will handle incoming messages and print/log them
void Client::msg_received(char *client_ip, char *msg){
 char *format = (char *)"msg from:%s\n[msg]:%s\n";
  char *cmd = (char *)"RECEIVED";

  shell_success(cmd);
  cse4589_print_and_log(format, client_ip, msg);
  shell_end(cmd);
}

/***************************
      HELPER FUNCTIONS
***************************/

/* isBlocked returns true if a client with client_ip is blocked, and returns
 * false otherwise */
bool Client::isBlocked(char *client_ip){
  std::list<client>::iterator it;
  for (it = blocked_clients.begin(); it != blocked_clients.end(); ++it) {
    client blocked_client = (*it);

  if (blocked_client.ip == client_ip) return true;
  }
  return false;
}