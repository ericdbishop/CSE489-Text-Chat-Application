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


/* logged_client struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */

/* each blocked_by structure contains the information of a client and a list of
 * every client they have blocked. This makes it easy to sort the list of
 * blocked clients using the compareClient() comparison. */

/* If we can't just use the compareClient comparison for our logged_client child
 * object then uncomment this code. */
//class comparelogged_client {
	//public:
	///* compareClient provides a sorting function for the connected_clients linked list */
	//inline bool operator()(const logged_client one, const logged_client two){ // Tutorials online added a & after the object type.
  //// This should work since logged_client inherits client
		//if (one.listening_port > two.listening_port) return false;
		//else return true;
	//}
//};

void Server::client_login(char *buffer){
	client *newClient = (client *)malloc(sizeof(client));

	receive_connected_client(buffer, newClient);

  // If the client is not already in the list of logged_clients, add it.
  //if (find(logged_clients.begin(), logged_clients.end(), newClient)
  logged_clients.insert(logged_clients.end(), logged_client(*newClient));
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
      //Check if the IP is valid
      if (!is_valid_ip(client_ip)) {
        // error
        perror("Invalid IP");
      }
      blocked(client_ip);
  }
 else return -1;

 return 0;
}

/* This function will send the list of connected clients to a client 
 * given the server object and the client socket number 
 * Returns 1 on success and -1 on failure */
void Server::send_connected_clients(int client_socket)
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
void Server::event(char *from_client_ip, char *to_client_ip, char *msg) {
  char *format = (char *)"%-5d%-35s%-8d%-8d%-8s\n";
  char *cmd = (char *)"RELAYED";

  shell_success(cmd);
  cse4589_print_and_log(format, from_client_ip, to_client_ip, msg);
  shell_end(cmd);

}