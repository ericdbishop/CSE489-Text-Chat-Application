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
#include <stdlib.h>
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
#include "../include/client.h"
#include "../include/server.h"

using namespace std;

#define EXIT_FAILURE 1
#define SERVER true
#define CLIENT false

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
	else if(strcmp(argv[1], "s") != 0 && strcmp(argv[1], "c") != 0) {
		printf(argv[1]);
		/* The double quotes allow you to split a string across multiple lines, I
		 * did this for readability. */
		cse4589_print_and_log("The first argument must be a 's' to initiate the" 
		"server or a 'c' to initiate the client\n");
	}
	else if(atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535) {
		cse4589_print_and_log("Please enter a valid port number in range 1024 <= x <= 65535\n");
	}

	/* Create the client/server object */
	if (strcmp(argv[1],"s") == 0) {
		Server program = Server(argv[2]);
		program.program_mode = SERVER;
		program.read_inputs();
	}
	else if (strcmp(argv[1], "c") == 0) {
		Client program = Client(argv[2]);
		program.program_mode = CLIENT;
		program.read_inputs();
	}
	return 0;
}

