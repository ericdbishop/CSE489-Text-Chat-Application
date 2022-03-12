/**
 * @ericbish_assignment1
 * @author  Eric Bishop <ericbish@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
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
#include "client.cpp"
#include "server.cpp"


using namespace std;

#define SERVER true
#define CLIENT false
bool program_mode; // stores whether the program is in server or client mode
//void program; // pointer to hold client or server object

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

	/*---------------------------Start Here---------------------------*/

	/* Verify Command Line Arguments */
	if(argc != 3) {
		cse4589_print_and_log("Incorrect Number of Arguments\n");
		exit(EXIT_FAILURE);
	}
	else if(argv[1] != "s" || argv[1] != "c") {
		cse4589_print_and_log("The first argument must be a 's' to initiate the server or a 'c' to initiate the client\n");
	}
	else if(atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535) {
		cse4589_print_and_log("Please enter a valid port number in range 1024 <= x <= 65535\n");
	}

	/* Create the client/server object */
	if (argv[1] == "s") {
		program_mode = SERVER;
		Server program = Server(atoi(argv[2]));
	}
	else if (argv[1] == "c") {
		program_mode = CLIENT;
		Client program = Client(atoi(argv[2]));
	}

	/*
		Get user input here, and make according function calls
	*/

	return 0;
}
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

void print_port() {
	char *cmd = "PORT";
	shell_success(cmd);
	if (program_mode == SERVER) {
		&(Server *)program.print_port(); // not sure if this is valid syntax, I will need to test it
	}
	else if (program_mode == CLIENT) {
		&(Client *)program.print_port();
	}
	shell_end(cmd);
}
