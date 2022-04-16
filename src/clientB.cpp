#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>
#include "constants.h"

using namespace std;

int main(int argc, char *argv[])
{
	// in case no argument received
	if(argc <= 1){
		cout << "Please type-in your cmd line argument" << endl;
		cout << "Client B ends with error: no argument" << endl;
		return 1;
	}

	// initiate, follow the instruction from GeekforGeek
	struct sockaddr_in serverM_addr;
	char buffer[1024] = {0};
	string request;
	string argv1, argv2;
	int sock;
	char* token;
    vector<string> response;

	// setup the socket using TCP protocol
	// connecting with server M
	sock = socket(PF_INET, SOCK_STREAM, 0);
	serverM_addr.sin_family = AF_INET;
    serverM_addr.sin_port = htons(Ports::serverM_B);

    // connect to server M at local host
    inet_pton(AF_INET, "127.0.0.1", &serverM_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serverM_addr, sizeof(serverM_addr));
    cout << "Client B is up and running" << endl;

    // create a request based on argv
    for(int i=1; i<argc; ++i)
    {
    	request += argv[i];
    	request += " ";
    }

    switch(argc)
    {
    	case 2:
    		argv1 = argv[1];
    		if(argv1.compare("TXLIST") == 0)
    		{
    			cout << "ClientB sent a sorted list request to the mainserver." << endl;

    			// add the dummy content so we can jump to case 4 on the server side
    			request += "dummy";
    			request += " ";
    			request += "dummy";
    			request += " ";
    			request += "dummy";
    			request += " ";

    			// send message and read response
		    	bzero(buffer, 1024);
			    send(sock, (char*) request.c_str(), strlen(request.c_str()), 0);
			    bzero(buffer, 1024);
			    recv(sock, buffer, 1024, 0);

			    // parse the response
			    token = strtok(buffer, " ");
	            while(token != NULL)
	            {
	            	// DEBUG MSG
				   	// cout << "DEBUG: Received response token: " << token << endl;
				   	// END DEBUG

	            	response.push_back(token);
	            	token = strtok(NULL, " ");
	            }

	            // print the response
	            if(response[0].compare("successed") == 0)
	            {
	            	cout << "alichain.txt is ready at the data folder" << endl;
	            } else {
	            	cout << "ClientB ends with unknown error" << endl;
	            }


    		} else {
    			cout << argv[1] << " sent a balance enquiry request to the main server." << endl;

		    	// send message and read response
		    	bzero(buffer, 1024);
			    send(sock, (char*) request.c_str(), strlen(request.c_str()), 0);
			    bzero(buffer, 1024);
			    recv(sock, buffer, 1024, 0);

			   	// parse the response
	            token = strtok(buffer, " ");
	            while(token != NULL)
	            {
	            	// DEBUG MSG
				   	// cout << "DEBUG: Received response token: " << token << endl;
				   	// END DEBUG

	            	response.push_back(token);
	            	token = strtok(NULL, " ");
	            }

			    // print the response
			    if(response[0].compare("invalid_usr") == 0)
			    {
			    	cout << "Unable to proceed with the request as " << argv[1] << \
			    	" is not part of the network " << endl;

			    } else if (response[0].compare("successed") == 0){
			    	cout << "The current balance of " << argv[1] << " is " << response[1]\
			     	<< " alicoins" << endl;

			    } else {
			    	cout << "Unknown Error" << endl;
			    }
    		}
    	break;

    	case 3:
    		argv2 = argv[2];
    		if(argv2.compare("stats") != 0)
    		{
    			cout << "Invalid request. Check if you miss spelled 'stats'" << endl;
    			cout << "Client B ends with error: invalid argument" << endl;
    			return 1;

    		}
    		cout << argv[1] << " sent a statistics enquiry request to the main server." << endl;

    		// send message and read response
	    	bzero(buffer, 1024);
		    send(sock, (char*) request.c_str(), strlen(request.c_str()), 0);
		    bzero(buffer, 1024);
		    recv(sock, buffer, 1024, 0);

		    // parse the response
            token = strtok(buffer, " ");
        	while(token != NULL)
            {
            	response.push_back(token);
            	token = strtok(NULL, " ");
            }

            if(response[0].compare("successed") == 0){
            	// DEBUG MSG
            	// cout << "DEBUG: response is " << buffer << endl;
            	// END DEBUG

            	cout << argv[1] << " statistics are the following" << endl;
            	// response >>;
            	cout << "Rank Username NumofTransacions Total" << endl;
            	int rank = 1;
            	for(vector<string>::size_type i = 1; i != response.size(); i+=3)
	            {
	            	cout << "  " << rank << "  ";
	            	cout << response[i] << "  ";
	            	cout << response[i+1] << "  ";
	            	cout << response[i+2] << "  " << endl;
	            	rank ++;
	            }
            	cout << "End of Table" << endl;
            } else if(response[0].compare("user_not_found") == 0) {
            	cout << "User " << argv[1] << " is not part of the network." << endl;

            } else {
            	cout << "Unknown Error" << endl;
            }

    	break;

    	case 4:
	    	cout << argv[1] << " has requested to transfer " << argv[3] << " coins to "\
	    	<< argv[2] << endl;

	    	// send message and read response
	    	bzero(buffer, 1024);
		    send(sock, (char*) request.c_str(), strlen(request.c_str()), 0);
		    bzero(buffer, 1024);
		    read(sock, buffer, 1024);
		    
		    // parse the response
            token = strtok(buffer, " ");
            while(token != NULL)
            {
            	// DEBUG MSG
			   	// cout << "DEBUG: Received response token: " << token << endl;
			   	// END DEBUG

            	response.push_back(token);
            	token = strtok(NULL, " ");
            }

		    // print the response
		    if(response[0].compare("insufficient_balance") == 0)
		    {
		    	cout << argv[1] << " was unable to transfer " << argv[3] << " alicoins to "\
			     	<< argv[2] << " because of insufficient balance" << endl;
			    cout << "The current balance of " << argv[1] << " is " << response[1] << " alicoins." << endl;

		    } else if (response[0].compare("invalid_sender") == 0) {
		    	cout << "Unable to proceed with the transaction as " << argv[1] << " is not part of the network"\
			    	<< endl;

		    } else if (response[0].compare("invalid_receiver") == 0) {
		    	cout << "Unable to proceed with the transaction as " << argv[2] << " is not part of the network"\
			    	<< endl;

		    } else if (response[0].compare("both_invalid") == 0) {
		    	cout << "Unable to proceed with the transaction as " << argv[1] << " and " \
		    		<< argv[2] << " are not part of the network" << endl;

		    } else if (response[0].compare("successed") == 0) {
		    	cout << argv[1] << " successfully transferred " << argv[3] << " alicoins to "\
			     	<< argv[2] << "." << endl;
			    cout << "The current balance of " << argv[1] << " is " << response[1] << " alicoins." << endl;

		    } else {

		    	cout << "Unknown Error" << endl;
		    }
    	break;
    }
    close(sock);
    return 0;
}