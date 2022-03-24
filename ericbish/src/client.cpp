#include "../include/global.h"
#include "../include/logger.h"
#include "../include/client.h"

Client::Client (char *port){
	//memset(&self, 0, sizeof(client));
	strcpy(self.listening_port, port);

	/* Fill in the details for the self Client object */
	makeClient(&self);

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
	socklen_t caddr_len;
  fdmax = 0;

	// clear the file descriptor sets
	FD_ZERO(&readfds);
	FD_ZERO(&master);

	FD_SET(STDIN, &master); // add stdin to the file descriptor set

 // This might not be neccesary in client
	/* Define listening socket value */
  create_listener(&self);

	listening_socket = self.listening_socket;
	FD_SET(listening_socket, &master); // add the client's listening socket to the file descriptor set
  if (listening_socket > fdmax)
		fdmax = listening_socket;

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
						// Process incoming data from server here ...
            char *msg = (char *)malloc(10);
            strcpy(msg, determine_msg_type(buffer));
							
            printf("Received: %s\n", buffer);
            /* If we are receiving a "client" message, it can be assumed that we
             * are getting a refresh or have just logged in. */
						if (strcmp(msg, "client") == 0) {
						  // requires_update should be set true when the client logs in or refreshes
						  if (requires_update) {
						  	// clear connected client list
						  	connected_clients.resize(0);
                requires_update = false;
              }
							client *newClient = (client *)malloc(sizeof(client));
							receive_connected_client(buffer, newClient);
						}						
            else if (strcmp(msg, "message") == 0) {
              msg_received(buffer);
            }

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

  if (cmd_and_arguments.find("REFRESH") != std::string::npos)
    refresh();

  else if (cmd_and_arguments.find("LOGOUT") != std::string::npos)
    logout();

  else if (cmd_and_arguments.find("EXIT") != std::string::npos)
    exit_server();

  else if (cmd_and_arguments.find("SEND") != std::string::npos) {
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

    send_msg(client_ip, msg);
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

    login(server_ip, server_port);
  }
  else if (cmd_and_arguments.find("UNBLOCK") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,7);
    string arguments = cmd_and_arguments.substr(8);
    char client_ip[arguments.size() + 1];

    arguments.copy(client_ip, arguments.length() + 1);
    client_ip[arguments.length()] = '\0';

    unblock(client_ip);
  }
  else if (cmd_and_arguments.find("BLOCK") != std::string::npos) {
    cmd = cmd_and_arguments.substr(0,5);
    string arguments = cmd_and_arguments.substr(6);
    char client_ip[arguments.size() + 1];

    arguments.copy(client_ip, arguments.length() + 1);
    client_ip[arguments.length()] = '\0';

    block(client_ip);
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
	if (getaddrinfo(server_ip, server_port, &hints, &res) != 0) {
    return -1;
  }

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0) {
		perror("Failed to create socket");
    return -1;
  }
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0) {
		perror("Connect failed");
    return -1;
  }
	
  /* Wait and then send information, as the server should be awaiting our information at this point*/
  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	buffer = package_client(self);

  usleep(100000);

  if (send(fdsocket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
    return -1;
  }

	freeaddrinfo(res);

	return fdsocket;
}

  
/* A client should not accept any other command, except LOGIN, EXIT, IP, PORT,
 * and AUTHOR, or receive packets, unless it is successfully logged in to the
 * server. */

/* Overload list() command to ensure that the client is logged in, and then
 * call parent function. */
void Client::list() {
  char *cmd = (char *)"LIST";
  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }
  Process::list();
}

void Client::login(char *server_ip, char *server_port){
  char *cmd = (char *)"LOGIN";

  if (logged_in){
    output_error(cmd);
    return;
  }

  server_socket = connect_to_host(server_ip, server_port);
  if (server_socket < 0) {
    output_error(cmd);
    exit(-1);
  }
  requires_update = true;

	FD_SET(server_socket, &master); // add server socket to fd set
  if (server_socket > fdmax)
		fdmax = server_socket;
  //printf("%d", server_socket);
  // now we can use server_socket to send and receive data

  // on login the server should send the client the list of currently connected clients

  // Maybe server should handle changing connected clients and the client should just request a new list of clients.
  logged_in = true;
  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
void Client::logout(){
  char *cmd = (char *)"LOGOUT";

  if (!logged_in){
    output_error(cmd);
    return;
  }

  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }

  close(server_socket);
	FD_CLR(server_socket, &master);

  // They should not be able to view connected clients and logged_in should be changed to false
  connected_clients.resize(0);
  logged_in = false;
  shell_success(cmd);
  shell_end(cmd);
  requires_update = false;
}

/* Retrieve an updated list of loggin in clients from the server and use it to update connected_clients */
void Client::refresh(){
  char *cmd = (char *)"REFRESH";
  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  memset(buffer, '\0', BUFFER_SIZE);

  // buffer structure: "refresh"|
  std::list<char *> segments;
  segments.insert(segments.end(), (char *)"refresh");

  strcpy(buffer, package(segments));

  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }

  requires_update = true;
  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
void Client::send_msg(char *client_ip, char *msg){
/* Exceptions:

 * Invalid IP address. 
 * Valid IP address which does not exist in the local copy
 * of the list of logged-in clients (This list may be outdated. Do not update
 * it as a result of this check). */

  char *cmd = (char *)"SEND";
  if (require_login(cmd) < 0) {
    printf("not logged in\n");
    output_error(cmd);
    return;
  }

  if (!is_valid_ip(client_ip)){
    printf("invalid ip %s\n", client_ip);
    output_error(cmd);
    return;
  }

  if (strlen(msg) > 256) {
    printf("length error\n");
    output_error(cmd);
    return;
  }

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  memset(buffer, '\0', BUFFER_SIZE);

  // messages structure: "message"|src_ip|dest_ip|msg
  std::list<char *> segments;
  segments.insert(segments.end(), (char *)"message");
  segments.insert(segments.end(), self.ip);
  segments.insert(segments.end(), client_ip);
  segments.insert(segments.end(), msg);

  strcpy(buffer, package(segments));

  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }

  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
void Client::broadcast(char *msg){
  char *cmd = (char *)"BROADCAST";
  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }

  if (strlen(msg) > 256) {
    printf("length error");
    output_error(cmd);
    return;
  }

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);

  char *broadcast_ip = (char *)"255.255.255.255";
  // messages structure: "message"|src_ip|dest_ip|msg
  std::list<char *> segments;
  segments.insert(segments.end(), (char *)"message");
  segments.insert(segments.end(), self.ip);
  segments.insert(segments.end(), broadcast_ip);
  segments.insert(segments.end(), msg);

  strcpy(buffer, package(segments));

  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }

  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
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
  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }

  /* Error if the given IP is already blocked */
  if (isBlocked(client_ip)){
    output_error(cmd);
    return;
  }

  if (!is_valid_ip(client_ip)){
    printf("invalid ip %s\n", client_ip);
    output_error(cmd);
    return;
  }

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);

  // block structure: msg_type|from_ip|block_ip|
  std::list<char *> segments;
  segments.insert(segments.end(), (char *)"block");
  segments.insert(segments.end(), self.ip);
  segments.insert(segments.end(), client_ip);

  strcpy(buffer, package(segments));

  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }

  blocked_clients.insert(blocked_clients.end(), client_ip);
  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
// unblock structure: msg_type|from_ip|unblock_ip|
void Client::unblock(char *client_ip){
/* Exceptions: */

  char *cmd = (char *)"UNBLOCK";
  if (require_login(cmd) < 0) {
    output_error(cmd);
    return;
  }

 /* Client with IP address: <client-ip> is not blocked.   */
  if (!isBlocked(client_ip)){
    output_error(cmd);
    return;
  }

 /* Invalid IP address. */
  if (!is_valid_ip(client_ip)){
    printf("invalid ip %s\n", client_ip);
    output_error(cmd);
    return;
  }

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);

  // unblock structure: msg_type|from_ip|unblock_ip|
  std::list<char *> segments;
  segments.insert(segments.end(), (char *)"unblock");
  segments.insert(segments.end(), self.ip);
  segments.insert(segments.end(), client_ip);

  strcpy(buffer, package(segments));

  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }

  blocked_clients.remove(client_ip);
  shell_success(cmd);
  shell_end(cmd);
}

// NEEDS TESTING
void Client::exit_server(){
  char *cmd = (char *)"EXIT";

  char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  strcpy(buffer, "exit|");
  if (send(server_socket, buffer, strlen(buffer), 0) == -1) {
    perror("send");
  }
  
  shell_success(cmd);
  shell_end(cmd);

  exit(0);
}

// FINISHED BUT UNTESTED
/* msgReceived will handle incoming messages that are still in the received
 * message buffer and print/log them */
void Client::msg_received(char *buffer){
  char client_ip[INET_ADDRSTRLEN], *msg;
  char *element_str; 
  std::list <char *> segments = unpack(buffer);

  // messages structure: "message"|src_ip|dest_ip|msg
  std::list<char *>::iterator it = segments.begin();
  // msg_type
  // src_ip
  it++;
  memset(client_ip, '\0', INET_ADDRSTRLEN);
  strcpy(client_ip, *it);

  //dest_ip
  it++;
  //msg
  it++;
  msg = (char *)malloc(strlen(*it));
  memset(msg, '\0', strlen(*it));
  strcpy(msg, *it);

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
  std::list<char *>::iterator it;
  for (it = blocked_clients.begin(); it != blocked_clients.end(); ++it) {
    char *blocked_client = (*it);

    if (strcmp(blocked_client, client_ip) == 0) return true;
  }
  return false;
}

/* Return -1 if client is not logged in, 0 if they are. */
int Client::require_login(char *cmd){
  if (!logged_in){
    shell_error(cmd);
    return -1;
  }
  return 0;
}