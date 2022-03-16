#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include "../include/global.h"
#include "../include/logger.h"

#include <list>

#define STDIN 0
#define CMD_SIZE 100
#define BACKLOG 5
#define BUFFER_SIZE 256

void shell_success(char *command_str); 
void shell_end(char *command_str);
void shell_error(char *command_str);
int makeClient(client *newClient);

/* Create a instance of the client struct for each client that connects. Maintain
 * a list of connected clients for all processes so that they can call list().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
struct client{
	int listening_port;
	char *ip;
	char hostname[128];
	/* compareClient provides a sorting function for the connected_clients linked list */
	bool operator()(const client one, const client two){
		if (one.listening_port > two.listening_port) return false;
		else return true;
	}
};

class Process {
  public:
	 //char hostname[128], ipstr[INET_ADDRSTRLEN]; // maybe INET_ADDR6STRLEN idk?? needs testing
	 struct client *self;
     // listening_socket is the socket fd.
     int listening_socket;
	 std::list<client> connected_clients;

  void create_listener(); 
  int read_inputs(); 

  Process (int port) {
	memset(&self, 0, sizeof(client));
	self->listening_port = port;

	/* Fill in the details for the self Client object */
	makeClient(self);
  }

  /* This helper function creates the socket we listen for new connections on,
   * it should be called during initialization of the Server
   *
   * We might want to put this in process.cpp, since the client creates a
   * listener socket as well
   */
  void create_listener() {
    int head_socket, selret, sock_index, fdaccept=0, caddr_len;
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
    listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(listening_socket < 0){
      perror("Cannot create socket");
    }
    
    /* Bind */
    if(bind(listening_socket, res->ai_addr, res->ai_addrlen) < 0 ){
      perror("Bind failed");
    }

    freeaddrinfo(res);
    
    /* Listen */
    if(listen(listening_socket, BACKLOG) < 0){
      perror("Unable to listen on port");
    }
  }

  int read_inputs(){
    struct sockaddr_in client_addr;
    fd_set readfds, master;
    int fdaccept, caddr_len, fdmax = 0;

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
            char *command = (char *)malloc(sizeof(char)*CMD_SIZE);
            memset(command, '\0', CMD_SIZE);
            if (fgets(command, CMD_SIZE-1, stdin) == NULL) {
              exit(-1);
            }

            // now we call the corresponding helper functions for each command
            if (strcmp(command, "AUTHOR") == 0) {
              author();
            }
            if (strcmp(command, "PORT") == 0) {
              port();
            }
            if (strcmp(command, "LIST") == 0) {
              list();
            }
            //... maybe move this somewhere so we can handle client commands too
            // or if the client needs to listen for connections from the server,
            // it can be moved back to 
          }
          else if (i == listening_socket) { // listener is the servers listening socket fd, 
                                            // I need to figure out how to store this in a variable
            // Accept new connections and add them to master set
            caddr_len = sizeof(client_addr);
						fdaccept = accept(listening_socket, (struct sockaddr *)&client_addr, &caddr_len);
						if(fdaccept < 0)
							perror("Accept failed.");
						
						printf("\nRemote Host connected!\n");                        
						
						/* Add to watched socket list */
						FD_SET(fdaccept, &master);
						if(fdaccept > fdmax) fdmax = fdaccept;
          }
          else {
            // handle data from a client
            /* Initialize buffer to receieve response */
						char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);
						
						if(recv(i, buffer, BUFFER_SIZE, 0) <= 0){
							close(i);
							printf("Remote Host terminated connection!\n");
							
							/* Remove from watched list */
							FD_CLR(i, &master);
						}
						else {
							//Process incoming data from existing clients here ...
							
							printf("\nClient sent me: %s\n", buffer);
							printf("ECHOing it back to the remote host ... ");
              // I'm pretty sure we don't want to use fdaccept when sending information to the clients
							if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
								printf("Done!\n");
							fflush(stdout);
						}
						
						free(buffer);
          }
        }
      }
    }
  }

/* SHELL commands */
  void output(char *cmd, char *format, char *input){
	  shell_success(cmd);
	  cse4589_print_and_log(format, input);
	  shell_end(cmd);
  }
  void output(char *cmd, char *format, int input){
	  shell_success(cmd);
	  cse4589_print_and_log(format, input);
	  shell_end(cmd);
  }

  void author() {
	  char *cmd = "AUTHOR";
	  char *format = "I, %s, have read and understood the course academic policy.\n";
	  char *name = "ericbish";
	 
	  output(cmd, format, name);
  }

  void ip() { // uses code from section 6.3 of Beej's Guide to Network Programming
		/* We still need to have some error handling in here, from how the PA1
		 * description is written. So we could maybe have a separate helper
		 * function that we call once when a process begins, to gather its
		 * hostname and external ip and whatnot, and then everytime the IP
		 * command is given we can call it again. The helper function will detect
		 * errors and return -1 if something is wrong. */

	  char *cmd = "IP";
	  char *format = "IP:%s\n";
		/* Will this prodcue issues if self has already been defined? */
      int result = makeClient(self);

	  if (result == -1){
		  shell_error(cmd);
		  return;
	  }
	  // Print output
	  output(cmd, format, self->ip);
  }

  void port() {
	  char *cmd = "PORT";
	  char *format = "PORT:%d\n";
	  output(cmd, format, self->listening_port);
  }

  /* list() should  */
  void list() {
	  int list_id, port_listen;
	  char *hname, *ip_addr;

	  char *cmd = "LIST";
	  shell_success(cmd);
	  int acc = 1;
	  for(auto i = connected_clients.begin(); i != connected_clients.end(); ++i) {
		// retrieve info for the next client in ascending port number order.
		client currentClient = (*i);
		list_id = acc;
		port_listen = currentClient.listening_port;
		hname = currentClient.hostname;
		ip_addr = currentClient.ip;
		acc++;
      	cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hname, ip_addr, port_listen);
	  }
	  shell_end(cmd);
	  
  }


}; // End Process class

/* Return 1 on success, -1 otherwise */
int makeClient(client *newClient){
    
	char *cmd = "IP";
	char ipstr[INET_ADDRSTRLEN]; // maybe INET_ADDR6STRLEN idk?? needs testing
	struct addrinfo hints, *res;
	int sockfd, status;

	// load up adress structs with getaddrinfo()
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((status = getaddrinfo("8.8.8.8", "53", &hints, &res)) != 0) {
	 // Do we need the same error checking here if it is outside of the IP command?
	 fprintf(stderr, "IP: getaddrinfo error: %s\n", gai_strerror(status));
	 return -1;
	}

	  // make a socket
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
	 fprintf(stderr, "IP: socket error\n");
	 return -1;
	}

	// connect
	if ((connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
	 fprintf(stderr, "IP: connect error\n");
	 return -1;
	}

	// get my IP Address
	struct sockaddr_in *myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	socklen_t len = sizeof(myaddr);
	if ((getsockname(sockfd, (struct sockaddr *) &myaddr, &len)) == -1) {
	 fprintf(stderr, "IP: getsockname error\n");
	 return -1;
	}

	if ((inet_ntop(AF_INET, &myaddr->sin_addr, ipstr, sizeof(ipstr))) == NULL ) {
	 fprintf(stderr, "IP: inet_ntop error\n");
	 return -1;
	}

	// close UDP socket
	if ((close(sockfd)) == -1) {
	 fprintf(stderr, "IP: close error\n:");
	 return -1;
	}

	newClient->ip = ipstr;
	/* The res addrinfo structure contains
	 * ai_canonname which should be the hostname. */
	std::strncpy(newClient->hostname, res->ai_canonname, sizeof(newClient->hostname));

	return 1;
};

/* Helper functions for SHELL output */
void shell_success(char *command_str) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
}

void shell_end(char *command_str) {
	cse4589_print_and_log("[%s:END]\n", command_str);
}

void shell_error(char *command_str) {
	cse4589_print_and_log("[%s:ERROR]", command_str);
	shell_end(command_str);
}