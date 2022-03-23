//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "../include/server.h"

Server::Server(char *port){
	//memset(&self, 0, sizeof(client));
	strcpy(self.listening_port, port);

	/* Fill in the details for the self Client object */
	makeClient(&self);

}

/* read_inputs() is responsible for calling all other functions and will run so
 * long as the program is running. Much of this is taken from/based off of the bgnet guide */
int Server::read_inputs(){

	struct sockaddr_in client_addr;
	socklen_t caddr_len;
  fdmax = 0;

	// clear the file descriptor sets
	FD_ZERO(&readfds);
	FD_ZERO(&master);

	FD_SET(STDIN, &master); // add stdin to the file descriptor set
	
	/* Define listening socket value */
  create_listener(&self);

	listening_socket = self.listening_socket;
	FD_SET(listening_socket, &master); // add stdin to the file descriptor set
  // We have to keep track of the biggest file descriptor
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
				else if (i == listening_socket) // listening socket fd
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

					// We should receive the clients information,
					// put that information into a client structure, then add the structure to the
					// connected clients list
				}
				else
				{
					// handle incoming data
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

            //decrement fdmax???

						// remove from connected clients list TODO
            client_logout(i);

					}
					else
					{
						// Process incoming data from existing clients here ...

            // Determine message type.
            /* Message types: client, message, refresh, block, unblock, exit?, logout? */ 
            char *msg = (char *)malloc(10);
            strcpy(msg, determine_msg_type(buffer));
            printf("Received message: %s \n", buffer);
            printf("msg type: %s \n", msg);

            // client structure: msg_type|listening_port|listening_socket|ip|hostname|
            if (strcmp(msg, "client") == 0) {
					    // 1. receive client information in a buffer - might need to wait a second im not sure
              printf("receiving login\n");
					    // 2. process the information using receive_connected_client
					    // Make sure client is accounted for in logged_clients.
					    client_login(buffer, i);
					    // newClient should be updated by receive_connected_client to contain the new client information.
					    // 4. send the complete list of connected clients to the client
					    send_connected_clients(i);

            // message structure: msg_type|msg|from_ip|to_ip|
            } else if (strcmp(msg, "message") == 0) {
              event(buffer, i);

            // refresh structure: msg_type|
            } else if (strcmp(msg, "refresh") == 0) {
					    send_connected_clients(i);

            // block structure: msg_type|from_ip|block_ip|
            } else if (strcmp(msg, "block") == 0) {

            // unblock structure: msg_type|from_ip|unblock_ip|
            } else if (strcmp(msg, "unblock") == 0) {

            // exit structure:: msg_type|sender_ip|
            } else if (strcmp(msg, "exit") == 0) {

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
void Server::send_connected_clients(int client_socket)
{
	// for each connected client send their information in a string with the format:
	// msg_type|listening_port|listening_socket|ip|hostname
  char *buffer;
  int len;
  client currentClient;

  std::list<client>::iterator it;
	for (it=connected_clients.begin(); it != connected_clients.end(); ++it) {
		buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
		client currentClient = (*it);
    	buffer = package_client(currentClient); 

    printf("buffer to send to client: %s", buffer);

		if(send(client_socket, buffer, strlen(buffer), 0) == -1){
      perror("send");
    }
	}
  // As far as I can tell there is not a reason to tell the client we are done sending.
	//buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  //strcat(buffer, "DONE\0");
  //len = strlen(buffer);
  //send(client_socket, buffer, len, 0);
}

/* The main purpose of client_login is so that when the server receives a login
 * they can decide if this client has already been previously logged into the
 * server */
void Server::client_login(char *buffer, int socket_for_send){
	client *newClient = (client *)malloc(sizeof(client));

	receive_connected_client(buffer, newClient);
  newClient->socket_for_send = socket_for_send;

  // If the client is not already in the list of logged_clients, add it.
  //if (find(logged_clients.begin(), logged_clients.end(), newClient)

  logged_client to_find = logged_client(*newClient);
	std::list<logged_client>::iterator find_result = find(&to_find);

  // Client is not logging in for the first time
  if (strcmp((*find_result).ip, to_find.ip) == 0){
    strcpy((*find_result).status, "logged-in"); 
  } else {
    // Client is logging in for first time
    logged_clients.insert(logged_clients.end(), to_find);
    logged_clients.sort(logged_client::operator());
  }

}

void Server::client_logout(int sock_fd){
	std::list<client>::iterator it;
  for (it=connected_clients.begin(); it != connected_clients.end(); ++it) {
	  client currentClient = (*it);
		char *client_sock = (char *)malloc(sizeof(char) * 6);// 6 because the max port number would be "65535\n"			sprintf(client_sock, "%d", currentClient.listening_socket);
		char *sock = (char *)malloc(sizeof(char) * 6);// 6 because the max port number would be "65535\n"			sprintf(sock, "%d", i);
    sprintf(sock, "%d", sock_fd);

   	if (strcmp(client_sock, sock) == 0){
   	  // client logging out						    	  ; 				}
    }
  }
}

int Server::call_command(char *command){
  if (Process::call_command(command) == 0) return 0;

  //Need to split the command up to separate the command from its arguments
  string cmd_and_arguments = std::string(command);
  size_t cmd_length = cmd_and_arguments.length();
  string cmd, arguments; 

  if (strcmp(command, "STATISTICS") == 0) 
    statistics();
  else if (cmd_length > 8) {
    cmd = cmd_and_arguments.substr(0,7);
    // Used to understand converting string type to a char *::w
    // https://www.tutorialspoint.com/how-to-convert-string-to-char-array-in-cplusplus 
    if (cmd.compare("BLOCKED") == 0)
      string arguments = cmd_and_arguments.substr(8);
      char client_ip[arguments.size() + 1];

      arguments.copy(client_ip, arguments.length() + 1);
      client_ip[arguments.length()] = '\0';

      blocked(client_ip);
  }
 else return -1;

 return 0;
}


/* This works the same as is_valid_ip in process.cpp, except it looks at
 * clients who are logged out and who are logged in. */
bool Server::is_valid_ip(char *client_ip){
  int acc = 1;
  std::list<logged_client>::iterator i;
  for (i = logged_clients.begin(); i != logged_clients.end(); ++i)
  {
  	// retrieve info for the next client in ascending port number order.
  	logged_client currentClient = (*i);
  	if (client_ip == currentClient.ip)
  		return true;
  }
  return false;
}


/* statistics displays a numbered list of clients who are or have previously
 * logged in to the server, who have not used the EXIT command, 
 * with statistics such as their number of sent and received
 * messages, and whether they are logged in or out. */
void Server::statistics(){
  logged_clients.sort(client());

 int list_id, num_msg_sent, num_msg_rcv;
 char *hname, *status;
 char *cmd = (char *)"STATISTICS";

 shell_success(cmd);
 int acc = 1;
 std::list<logged_client>::iterator i;
 for(i = logged_clients.begin(); i != logged_clients.end(); ++i) {
  // retrieve info for the next client in ascending port number order.
  logged_client currentClient = (*i);
  list_id = acc;
  hname = currentClient.hostname;
  num_msg_sent = currentClient.num_msg_sent;
  num_msg_rcv = currentClient.num_msg_rcv;
  status = currentClient.status;
  acc++;
    cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_id, hname, num_msg_sent, num_msg_rcv, status);
 }
 shell_end(cmd);
}

/* blocked handles the BLOCKED command, which displays a list of client info
 * for each client blocked by the client who's ip is given as an argument. The
 * output is formatted the same as the list() command and sorted by ascending
 * port numbers. */
void Server::blocked(char *client_ip) {
  char *cmd = (char *)"BLOCKED";
  int list_id, port_listen;
  char *hname, *ip_addr;

  if (!is_valid_ip(client_ip)){
    printf("invalid ip %s", client_ip);
    output_error(cmd);
    return;
  }

  /* Iterate over clients in the block_lists until we find the one with the
   * right ip. */
  std::list<blocked_by>::iterator i;
  for (i = block_lists.begin(); i != block_lists.end(); ++i) {
    blocked_by current_client = (*i);

    if (current_client.ip == client_ip) {
      shell_success(cmd);
      current_client.blocked.sort(client());

      /* Iterate over the sorted list of clients that are blocked and print
       * them with the same format as the list() command */
      int acc = 1;
      std::list<client>::iterator it;
      for (it = current_client.blocked.begin(); it != current_client.blocked.end(); ++it) {
        client blocked_client = (*it);

        list_id = acc;
        port_listen = atoi(blocked_client.listening_port);
        hname = blocked_client.hostname;
        ip_addr = blocked_client.ip;
        acc++;
    	  cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hname, ip_addr, port_listen);
      }

      shell_end(cmd);
    }
  }
}

/* The event function will handle output when a client sends a message 
 * which is routed through the server. In the case of a broadcast message,
 * the to_client_ip should be 255.255.255.255 */
void Server::event(char *buffer, int sender) {
  char *from_client_ip, *to_client_ip, *msg;
  char *format = (char *)"%-5d%-35s%-8d%-8d%-8s\n";
  char *cmd = (char *)"RELAYED";
  char *broadcast_ip = (char *)"255.255.255.255";
  std::list<char *> segments = unpack(buffer);

  std::list<char *>::iterator it = segments.begin();
  it++;

  // messages structure: "message"|src_ip|dest_ip|msg
  from_client_ip = (*(it++));
  to_client_ip = (*(it++));
  msg = (char *)malloc(strlen());
  msg = (*it);

  int sock;
  bool BROADCAST;
  BROADCAST = strcmp(broadcast_ip, to_client_ip) == 0;
	std::list<logged_client>::iterator it;

  for (it=logged_clients.begin(); it != logged_clients.end(); ++it) {
	  logged_client currentClient = (*it);
    sock = currentClient.socket_for_send;

	  if (sock != sender) {  // If client is not the sender
      if (BROADCAST || strcmp(currentClient.ip, to_client_ip) == 0) { // BROADCAST or client is found in the list of logged clients
        // && FD_ISSET(sock, &master)
        if (strcmp(currentClient.status, "logged-in") == 0) { // If this client is logged-in, send the message.
          if (send(sock, buffer, nbytes, 0) == -1) {
            perror("send");
          } else {
            currentClient.num_msg_rcv += 1; // Increment the number of messagese received.
          }
        } else { // Client is logged out, so buffer the message for them
          currentClient.buffered_messages.insert(currentClient.buffered_messages.end(), buffer);
        }
      } 
	  } else currentClient.num_msg_sent += 1; // Increment sender's messages sent
  } 

  shell_success(cmd);
  cse4589_print_and_log(format, from_client_ip, to_client_ip, msg);
  shell_end(cmd);

}

// These functions iterate through either logged_clients or connected_clients to
// find out if it exists. If it is not found, it returns the first element of
// the list.
list<logged_client>::iterator Server::find(logged_client *to_find){
	std::list<logged_client>::iterator it;
  for (it=logged_clients.begin(); it != logged_clients.end(); ++it) {
	logged_client currentClient = (*it);
	  // if client is found in the list of logged clients
	  if (strcmp(currentClient.ip, to_find->ip) == 0)
      return it;
	}
  return logged_clients.begin();
}
list<client>::iterator Server::find(client *to_find){
	std::list<client>::iterator it;
  for (it=connected_clients.begin(); it != connected_clients.end(); ++it) {
	  client currentClient = (*it);
	  // if client is found in the list of logged clients
	  if (strcmp(currentClient.ip, to_find->ip) == 0)
      return it;
	}
  return connected_clients.begin();
}