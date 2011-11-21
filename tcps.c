/* 
	File: tcps.c

	A simple TCP server.
	Sends a generic message as a response.
	Listens on a specified port.
	Delegates clients to child processes.
	
	Author: Nick Pink
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Print message to standard error, exit program with error
void error(const char *msg) {
	perror(msg);
	exit(1);	
}

// Child servers indefinitely respond to messages from their client
void handle_client (int client) {
    char const welcome[45] = "Hello. I'm a TCP server.  What's going on?\n";
    char const response[60] = "Tell me about it. What else is happening?\n";
    char const quit_sig[4] = "quit";
    int success;
    char buffer[256]; // Read buffer
    
    success = write(client, welcome, sizeof(welcome));
	if (success < 0) {
		error("Unable to write to socket");
	}
    
    // This is an infinite loop
    do {
        bzero(buffer,256); // Clear buffer for next read
    	// Read up to 255 characters
    	success = read(client, buffer, 255);
    	if (success < 0) {
    		error("Unable to read from socket");
    	}
    	printf("The client says: %s", buffer);
	
    	// Send a response message to client
    	success = write(client, response, sizeof(response));
    	if (success < 0) {
    		error("Unable to write to socket");
    	}
    } while (1);
}

// Main server process
int main(int argc, char* argv[]) {
	if (argc != 2 ) {
		printf("\nUsage:\ntcps <port>\n\n");
		exit(0);
	}
	
	// Convert port argument to a number
	int port = strtol(argv[1], NULL, 10);
	int a_client, success, pid;	
	// Structures that hold server and client addresses
	struct sockaddr_in serv_addr, cli_addr;
	int cli_size = sizeof(cli_addr);
	// Create Internet streaming socket
	int gateway = socket(AF_INET, SOCK_STREAM, 0);
	
	if (gateway < 0) {
		error("Unable to open socket");
	}
	printf("Opened a gateway socket.\n");
	
	// Set up our address //
	// Initialize structure to zero bytes
	bzero((char *) &serv_addr, sizeof(serv_addr));	
	serv_addr.sin_family = AF_INET; // Internet socket	
	serv_addr.sin_port = htons(port); // To network byte order
	// Only accept connections from localhost	
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// Bind socket to our address
	success = bind(
		gateway, 
		(struct sockaddr*) &serv_addr, 
		sizeof(serv_addr)
	);
	if (success < 0) {
		error("Unable to bind to port");	
	}
	printf("Bound to port %i on localhost\n", port);
	
	// Listen for incoming connections
	// Allow at most 5 connections to wait
	listen(gateway, 5);
	printf("Listening...\n");
	
	// Infinite loop
	while (1) {
	    // Someone wants to talk to us!
        a_client = accept(
			gateway, 
			(struct sockaddr *) &cli_addr, 
			&cli_size
		);
		if (a_client < 0) {
			error("Unable to accept client");
		} 
		printf("A client just connected. Opened another socket.\n");
        
        // Create a child server to handle this client
        pid = fork();
        if (pid < 0) {
			 error("Unable to create child server");
		}
		// We are in the child     
        if (pid == 0)  {
            // We only care about our client here
            close(gateway);
            handle_client(a_client);
            exit(0); // Child's job is done!
        } 
        // We must be in the parent
        else {
            printf("Client delegated to child server\n");
            // This is the child's problem now
            close(a_client);
        }
    } // Endlessly wait for more clients
	
	// We will never get here, but syntax requires this
	return 0;
}