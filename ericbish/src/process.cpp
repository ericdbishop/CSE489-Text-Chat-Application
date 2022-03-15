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
};

class compareClient {
	public:
	/* compareClient provides a sorting function for the connected_clients linked list */
	inline bool operator()(const client one, const client two){ // Tutorials online added a & after the object type.
		if (one.listening_port > two.listening_port) return false;
		else return true;
	}
};

class Process {
	public:
     int port_listen;
	 //char hostname[128], ipstr[INET_ADDRSTRLEN]; // maybe INET_ADDR6STRLEN idk?? needs testing
	 struct client *self;
	 std::list<client> connected_clients;

  Process (int port) {
    port_listen = port;
	/* The addrinfo structure that we get in the call to ip() contains
	 * ai_canonname which should be the hostname. Can we call getaddrinfo just
	 * one time in the constructor of the class and then just reference it
	 * elsewhere? */

	/* This following line of commented out code is probably not necessary, but
	 * we should be establishing the hostname and the ip within this
	 * constructor. */

	memset(&self, 0, sizeof(client));
	self->listening_port = port;

	/* Fill in the details for the self Client object and add it to the
	 * connected_clients list. */
	makeClient(self);
	connected_clients.insert(connected_clients.begin(), *self);
	// Sorting isn't neccesary here if it self is the only client in the list
	// but for future reference this is how we sort:
	// connected_clients.sort(compareClient());
  }

/* SHELL commands */

	/*
		Can we just hardcode the name to equal one or both of our ubit names?
	*/
  void author() {
	  char *cmd = "AUTHOR";
	  shell_success(cmd);
	  // I am assuming this should be ericbish since I generated the starter code.
	  char *name = "ericbish";
	  cse4589_print_and_log("I, %s, have read and understood the course academic policy.\n", name);
	  shell_end(cmd);
  }

  void ip() { // uses code from section 6.3 of Beej's Guide to Network Programming
	  char *cmd = "IP";

		/* We still need to have some error handling in here, from how the PA1
		 * description is written. So we could maybe have a separate helper
		 * function that we call once when a process begins, to gather its
		 * hostname and external ip and whatnot, and then everytime the IP
		 * command is given we can call it again. The helper function will detect
		 * errors and return -1 if something is wrong. */

		/* Will this prodcue issues if self has already been defined? */
      int result = makeClient(self);

	  if (result == -1){
		  shell_error;
		  return;
	  }

	  // Print output
	  shell_success(cmd);
	  cse4589_print_and_log("IP:%s\n", self->ip);
	  shell_end(cmd);
  }

  void print_port() {
	  char *cmd = "PORT";
	  shell_success(cmd);
      cse4589_print_and_log("PORT:%d\n", port_listen);
	  shell_end(cmd);
  }

  /* list() should  */
  void list() {
	  int list_id, port_listen;
	  char *hname, *ip_addr;

	  char *cmd = "LIST";
	  shell_success(cmd);
	  // Obviously this print and log needs to be done for every connected host.
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

};

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
	// This should make hostname equal to ai_canonname
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