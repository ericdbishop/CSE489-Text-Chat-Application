#include <iostream>
#include <stdio.h>

#include "../include/global.h"
#include "../include/logger.h"

#include "process.cpp"

/* logged_client struct contains statistics about each previously loggin in client. 
 * Maintain a list of logged clients for server so that it can call statistics().
 * We should actively maintain the correct order of clients so it goes from
 * smallest to largest port number */
struct logged_client:client {
	int num_msg_sent, num_msg_rcv;
  char *status;
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

void statistics();
void blocked(char client_ip);
void event();

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
};
