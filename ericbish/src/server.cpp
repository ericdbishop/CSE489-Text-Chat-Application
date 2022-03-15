#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

#define STDIN 0
#define MSG_SIZE 256
#define BACKLOG 5

/* logged_client struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
struct logged_client:client {
	int num_msg_sent, num_msg_rcv;
  char *status;
};

/* each blocked_by structure contains the information of a client and a list of
 * every client they have blocked. This makes it easy to sort the list of
 * blocked clients using the compareClient() comparison. */
struct blocked_by:client {
  std::list<client> blocked;
};

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

class Server: public Process {
  public:
	std::list<logged_client> logged_clients;
  std::list<blocked_by> block_lists;

void statistics();
void blocked(char *client_ip);
void event(char *from_client_ip, char *to_client_ip, char *msg, bool broadcast);

/* I think the following line of code is redundant, according to one guide, the
 * parent constructor of Process will be called automatically */
//using Process::Process;

  // The process constructor should be called first.
  Server (int port) : Process(port) {
    // Server constructor
  }

  /* statistics displays a numbered list of clients who are or have previously
   * logged in to the server, who have not used the EXIT command, 
   * with statistics such as their number of sent and received
   * messages, and whether they are logged in or out. */
  void statistics(){
    logged_clients.sort(compareClient());

	  int list_id, num_msg_sent, num_msg_rcv;
	  char *hname, *status;
	  char *cmd = "STATISTICS";

	  shell_success(cmd);
	  int acc = 1;
	  for(auto i = logged_clients.begin(); i != logged_clients.end(); ++i) {
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
  void blocked(char *client_ip) {
    char *cmd = "BLOCKED";
	  int list_id, port_listen;
	  char *hname, *ip_addr;

    /* Iterate over clients in the block_lists until we find the one with the
     * right ip. */
    for (auto i = block_lists.begin(); i != block_lists.end(); ++i) {
      blocked_by current_client = (*i);

      if (current_client.ip == client_ip) {
        shell_success(cmd);
        current_client.blocked.sort(compareClient());

        /* Iterate over the sorted list of clients that are blocked and print
         * them with the same format as the list() command */
	      int acc = 1;
        for (auto it = current_client.blocked.begin(); it != current_client.blocked.end(); ++it) {
          client blocked_client = (*it);

		      list_id = acc;
		      port_listen = blocked_client.listening_port;
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
  void event(char *from_client_ip, char *to_client_ip, char *msg) {
	  char *format = "%-5d%-35s%-8d%-8d%-8s\n";
    char *cmd = "RELAYED";

    shell_success(cmd);
    cse4589_print_and_log(format, from_client_ip, to_client_ip, msg);
    shell_end(cmd);

  }

  /* This helper function creates the socket we listen for new connections on,
   * it should be called during initialization of the Server
   */
  int create_listener() {
    int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	  struct sockaddr_in client_addr;
	  struct addrinfo hints, *res;
	  fd_set master_list, watch_list;

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

    /* Fill up address structures */
    if (getaddrinfo(NULL, to_string(self->listening_port).c_str(), &hints, &res) != 0){
      perror("getaddrinfo failed");
    }
    
    /* Socket */
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(server_socket < 0){
      perror("Cannot create socket");
    }
    
    /* Bind */
    if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 ){
      perror("Bind failed");
    }

    freeaddrinfo(res);
    
    /* Listen */
    if(listen(server_socket, BACKLOG) < 0){
      perror("Unable to listen on port");
    }
  }

  int read_inputs() {
    fd_set readfds, master;
    int fdmax = 0;

    // clear the file descriptor sets
    FD_ZERO(&readfds);
    FD_ZERO(&master);

    FD_SET(STDIN, &master); // add stdin to the file descriptor set

    while(true) {
      readfds = master;
      if(select(fdmax+1, &readfds, NULL, NULL, NULL) == -1) {
        fprintf(stderr, "select error\n");
        // the code in the book makes a call to exit(4)
        // not sure if this is how we should handle the error though
      }
      for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &readfds)) { // found a file descriptor
          if (i == STDIN) {
            // HANDLE SHELL COMMANDS

            // first we read the command from the command line
            char *command = (char *)malloc(sizeof(char)*MSG_SIZE);
            memset(command, '\0', MSG_SIZE);
            if (fgets(command, MSG_SIZE-1, stdin) == NULL) {
              exit(-1);
            }

            // now we call the corresponding helper functions
            if (strcmp(command, "AUTHOR") == 0) {
              program.author();
            }
            if (strcmp(command, "PORT") == 0) {
              program.print_port();
            }
            //... maybe move this somewhere so we can handle client commands too
          }
          else if (i == listener) { // listener is the servers listening socket fd, I need to figure out how to store this in a variable
            // accept new connections and add them to master set
          }
          else {
            // handle data from a client
          }
        }
      }
    }
  }

};
