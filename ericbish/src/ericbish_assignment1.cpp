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

/*
 * Partner: Brett Sitzman
 */

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
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

 
// void program; // pointer to hold client or server object

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
		/* The double quotes allow you to split a string across multiple lines, I
		 * did this for readability. */
		cse4589_print_and_log("The first argument must be a 's' to initiate the" 
		"server or a 'c' to initiate the client\n");
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
		The rest of this file should handle reading any further inputs from the
		user and feeding them into the object for the Client or Server object.
	*/
	fd_set readfds, master;
	int fdmax;

	// clear the file descriptor sets
	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(STDIN, &master); // add stdin to the file descriptor set

	while(true) {
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "select error\n");
			// the code in the book makes a call to exit(4)
			// not sure if this is how we should handle the error though
		}
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // found a file descriptor
				if (i == STDIN) {
					// HANDLE SHELL COMMANDS
				}
				else if (i == listener && program_mode == SERVER) { // listener is the servers listening socket fd, I need to figure out how to store this in a variable
					// accept new connections and add them to master set
				}
				else {
					// handle data from a client
				}
			}
		}
	}


	return 0;
}
