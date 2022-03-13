#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include "../include/global.h"
#include "../include/logger.h"


class Process {
public:
    int port_listen;

  Process (int port) {
    port_listen = port;
  }

/* SHELL commands */
  void author(char *name) {
	  char *cmd = "AUTHOR";
	  shell_success(cmd);
	  cse4589_print_and_log("I, %s, have read and understood the course academic policy.\n", name);
	  shell_end(cmd);
  }

  void ip() { // uses code from section 6.3 of Beej's Guide to Network Programming
	  char *cmd = "IP";
	  int sockfd;
	  struct addrinfo hints, *res;

	  // load up adress structs with getaddrinfo()
	  memset(&hints, 0, sizeof(hints));
	  hints.ai_family = AF_INET;
	  hints.ai_socktype = SOCK_DGRAM;

	  if ((getaddrinfo("8.8.8.8", "53", &hints, &res)) != 0) {
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

	  /*
		  Should it be sockaddr_in* instead of sockaddr_in?
	  */

	  // get my IP Address
	  struct sockaddr_in myaddr;
	  memset(&myaddr, 0, sizeof(myaddr));
	  socklen_t len = sizeof(myaddr);
	  if ((getsockname(sockfd, (struct sockaddr *) &myaddr, &len)) == -1) {
		  fprintf(stderr, "IP: getsockname error\n");
		  shell_error(cmd);
		  return;
	  }
	  char ipstr[INET_ADDRSTRLEN]; // maybe INET_ADDR6STRLEN idk?? needs testing

	  /*
		  ERROR because myaddr is a sockaddr_in type instead of a pointer type
	  */
	  if ((inet_ntop(AF_INET, myaddr->sin_addr, ipstr, sizeof(ipstr))) == NULL ) {
		  fprintf(stderr, "IP: inet_ntop error\n");
		  shell_error(cmd);
		  return;
	  }

	  /*
		  ERROR resulting from calling close(). I think it is due to including
		  fstream instead of fstream.h but fstream.h produces a separate error.

		  https://www.ibm.com/docs/en/zvse/6.2?topic=SSB27H_6.2.0/fa2ti_call_close.html
	  */

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

/*
    print_port should be overloaded in client and server and probably left
    undefined here. Obviously there is no program_mode or program variables in
    this file.
*/
  void print_port() {
	  char *cmd = "PORT";
	  shell_success(cmd);
      cse4589_print_and_log("PORT:%d\n", port_listen);
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