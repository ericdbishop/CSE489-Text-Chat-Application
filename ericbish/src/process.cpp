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

void shell_success(char *command_str); 
void shell_end(char *command_str);
void shell_error(char *command_str);


/* Create a instance of the client struct for each client that connects. Maintain
 * a list of connected clients for all processes so that they can call list().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number*/
struct client{
	int listening_port;
	char *ip;
	char *hostname[128];
};

class Process {
public:
    int port_listen;
	char *hostname[128];

	/* I made connected_clients an array of 5 because there are five dedicated
	 * hosts on the cse servers. */
	client connected_clients[5];

  Process (int port) {
    port_listen = port;
	/* The addrinfo structure that we get in the call to ip() contains
	 * ai_canonname which should be the hostname. Can we call getaddrinfo just
	 * one time in the constructor of the class and then just reference it
	 * elsewhere? */

	/* This following line of commented out code is probably not necessary, but
	 * we should be establishing the hostname and the ip within this
	 * constructor. */

	//int gethostname(hostname, sizeof(hostname));
  }

/* SHELL commands */

	/*
		Can we just hardcode the name to equal one or both of our ubit names?
	*/
  void author(char *name) {
	  char *cmd = "AUTHOR";
	  shell_success(cmd);
	  cse4589_print_and_log("I, %s, have read and understood the course academic policy.\n", name);
	  shell_end(cmd);
  }

  void ip() { // uses code from section 6.3 of Beej's Guide to Network Programming
	  char *cmd = "IP";
	  int sockfd, status;
	  struct addrinfo hints, *res;

	  // load up adress structs with getaddrinfo()
	  memset(&hints, 0, sizeof(hints));
	  hints.ai_family = AF_INET;
	  hints.ai_socktype = SOCK_DGRAM;

	  if ((status = getaddrinfo("8.8.8.8", "53", &hints, &res)) != 0) {
		  fprintf(stderr, "IP: getaddrinfo error: %s\n", gai_strerror(status));
		  shell_error(cmd);
		  return;
	  }

	  // make a socket
	  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		  fprintf(stderr, "IP: socket error\n");
		  shell_error(cmd);
		  return;
	  }

	  // connect
	  if ((connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
		  fprintf(stderr, "IP: connect error\n");
		  shell_error(cmd);
		  return;
	  }

	  // get my IP Address
	  struct sockaddr_in *myaddr;
	  memset(&myaddr, 0, sizeof(myaddr));
	  socklen_t len = sizeof(myaddr);
	  if ((getsockname(sockfd, (struct sockaddr *) &myaddr, &len)) == -1) {
		  fprintf(stderr, "IP: getsockname error\n");
		  shell_error(cmd);
		  return;
	  }
	  char ipstr[INET_ADDRSTRLEN]; // maybe INET_ADDR6STRLEN idk?? needs testing

	  if ((inet_ntop(AF_INET, &myaddr->sin_addr, ipstr, sizeof(ipstr))) == NULL ) {
		  fprintf(stderr, "IP: inet_ntop error\n");
		  shell_error(cmd);
		  return;
	  }

	  // close UDP socket
	  if ((close(sockfd)) == -1) {
		  fprintf(stderr, "IP: close error\n:");
		  shell_error(cmd);
		  return;
	  }

	  // Print output
	  shell_success(cmd);
	  cse4589_print_and_log("IP:%s\n", ipstr);
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
	  int list_id;
	  /* The following line should be removed once we are storing the ip_addr in
	   * in a client structure within our list of client structures. */
	  char *ip_addr;

	  char *cmd = "LIST";
	  shell_success(cmd);
	  // Obviously this print and log needs to be done for every connected host.
      cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hostname, ip_addr, port_listen);
	  shell_end(cmd);
	  
  }

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