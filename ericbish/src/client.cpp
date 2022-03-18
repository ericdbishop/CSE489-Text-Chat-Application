#include "../include/global.h"
#include "../include/logger.h"
#include "../include/client.h"

Client::Client (char *port) : Process(port) {
  // Add self Client object to list of connected clients.
  logged_in = false;
  connected_clients.insert(connected_clients.begin(), *self);

 // Sorting isn't neccesary here if self is the only client in the list
 // but for future reference this is how we sort:
 // connected_clients.sort(compareClient());
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
    exit();
  
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

    //Check if the server IP and port is correct
    if (strcmp(s_ip, server_ip) != 0) {
      perror("Invalid IP");
    } else if (strcmp(s_port, server_port) != 0) {
      perror("Invalid port");
    }

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
 return -1;

 return 0;
}

// helper function for LOGIN command - taken from client.c
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
	
	freeaddrinfo(res);

	return fdsocket;
}


void Client::require_login(char *cmd){
  if (!logged_in){
    shell_error(cmd);
  }
}
/* Invalid IP address. Valid IP address which does not exist in the local copy
 * of the list of logged-in clients (This list may be outdated. Do not update
 * it as a result of this check). Client with IP address: <client-ip> is already blocked. */
  
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
  }

  // now we can use server_socket to send and receive data

  // on login the server should send the client the list of currently connected clients
}

/* Retrieve an updated list of loggin in clients from the server and use it to update connected_clients */
void Client::refresh(){
  char *cmd = (char *)"REFRESH";
  require_login(cmd);
}

void Client::send(char *client_ip, char *msg){
  char *cmd = (char *)"SEND";
  require_login(cmd);
}

void Client::broadcast(char *msg){
  char *cmd = (char *)"BROADCAST";
  require_login(cmd);

}

void Client::block(char *client_ip){
  char *cmd = (char *)"BLOCK";
  require_login(cmd);

  if (isBlocked(client_ip)){
    shell_error(cmd);
  }

  /* TO DO: NOTIFY SERVER */


}

void Client::unblock(char *client_ip){
  char *cmd = (char *)"UNBLOCK";
  require_login(cmd);

  /* TO DO: NOTIFY SERVER */


}

void Client::logout(){
  char *cmd = (char *)"LOGOUT";
  require_login(cmd);

}

void Client::exit(){
  char *cmd = (char *)"EXIT";

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