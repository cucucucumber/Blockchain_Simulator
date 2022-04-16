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
#include <algorithm>
#include <vector>
#include <ctime>
#include <fstream>
#include <map>
#include "constants.h"

using namespace std;

int main(int argc, char *argv[])
{
	// we follow the instructions from Beej's Guide
	struct sockaddr_in servers_addr;
	struct sockaddr_in clientA_addr;
	struct sockaddr_in clientB_addr;
	struct sockaddr_in backend_addr;
	struct sockaddr_in client_addr;
	int clientA_fd, clientB_fd, servers_fd, backend_fd, client_fd;
	int newfd, maxfd, wallet, rand_pick;
	int last_serial = 0;
	char buffer[1024] = {0};
	bool found, invalid_receiver, invalid_sender;
	string client_name;
	string return_msg;
	string backend_msg;
	string converted;
	fd_set master;
	fd_set read_fds;
	socklen_t addrlen;

	// initialize the socket for servers and clients
	backend_fd = socket(PF_INET, SOCK_DGRAM, 0);
	servers_fd = socket(PF_INET, SOCK_DGRAM, 0);
	clientA_fd = socket(PF_INET, SOCK_STREAM, 0);
	clientB_fd = socket(PF_INET, SOCK_STREAM, 0);


	// bind the sockets
	addrlen = sizeof(servers_addr);

	// set the port numbers
	servers_addr.sin_family = AF_INET;
	servers_addr.sin_port = htons(Ports::serverM_S);
	servers_addr.sin_addr.s_addr = INADDR_ANY;
	memset(servers_addr.sin_zero, '\0', sizeof servers_addr.sin_zero);

	clientA_addr.sin_family = AF_INET;
	clientA_addr.sin_port = htons(Ports::serverM_A);
	clientA_addr.sin_addr.s_addr = INADDR_ANY;
	memset(clientA_addr.sin_zero, '\0', sizeof clientA_addr.sin_zero);

	clientB_addr.sin_family = AF_INET;
	clientB_addr.sin_port = htons(Ports::serverM_B);
	clientB_addr.sin_addr.s_addr = INADDR_ANY;
	memset(clientA_addr.sin_zero, '\0', sizeof clientA_addr.sin_zero);

	// bind the sockets
	bind(servers_fd, (struct sockaddr *)&servers_addr, addrlen);
	bind(clientA_fd, (struct sockaddr *)&clientA_addr, addrlen);
	bind(clientB_fd, (struct sockaddr *)&clientB_addr, addrlen);

	// listen 
	listen(clientA_fd, 3);
	listen(clientB_fd, 3);
	listen(servers_fd, 3);

	// finished initalization
	cout << "The main server is up and running" << endl;
	FD_ZERO(&master);
    FD_ZERO(&read_fds);

    maxfd = max(servers_fd, clientA_fd);
    maxfd = max(maxfd, clientB_fd);

	while(1)
	{
		 // set the three file descriptors
		FD_SET(servers_fd, &master);
	    FD_SET(clientA_fd, &master);
	    FD_SET(clientB_fd, &master);

        // select the ready descriptor
        select(maxfd+1, &master, NULL, NULL, NULL);

        // when any client is ready
        if (FD_ISSET(clientA_fd, &master) || FD_ISSET(clientB_fd, &master)) 
        {
        	if(FD_ISSET(clientA_fd, &master))
        	{ 
        		client_name = "A";
        		client_fd = clientA_fd;
        		client_addr = clientA_addr;
    		} 
    		else 
    		{
    			client_name = "B";
        		client_fd = clientB_fd;
        		client_addr = clientB_addr;
    		}

            newfd = accept(client_fd, (struct sockaddr*)&client_addr, &addrlen);
            bzero(buffer, sizeof(buffer));

            // receive client command
            read(newfd, buffer, sizeof(buffer));
            char* token;
            vector<string> requests;
            token = strtok(buffer, " ");
            while(token != NULL)
            {
            	requests.push_back(token);
            	token = strtok(NULL, " ");
            }

            // handle the client
            switch(requests.size())
            {
            	// CHECK_WALLET()
            	case 1:
            		found = false;
	            	wallet = 1000;
	            	cout << "The main server received input=" << requests[0] \
	            	<< " from the client using TCP over port " << ntohs(client_addr.sin_port) << endl;
	            	for (int i=0; i<3; ++i)
	            	{
	            		// setup backend port number
	            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
	            		backend_addr.sin_family = AF_INET;
						backend_addr.sin_addr.s_addr = INADDR_ANY;
						memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

						// DEBUG MSG
						// cout << "DEBUG: the received buffer is " << buffer << endl;
						// END DEBUG

						// build the backend request
						backend_msg = "check_wallet";
						backend_msg += " ";
						backend_msg += requests[0];
						backend_msg += " ";

						// DEBUG MSG
						// cout << "DEBUG: backend_msg is " << backend_msg << endl;
						// END DEBUG

						// contact backend server
	            		cout << "The main server sent a request to server " << Ports::convert[i] << endl;
	            		sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
			            bzero(buffer, sizeof(buffer));
			            recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
			            converted = buffer;
			            cout << "The main server received transactions from Server " << Ports::convert[i] \
			            <<" using UDP over port " << Ports::backend_ports[i] << endl;

			            // DEBUG MSG
			            // cout << "DEBUG: The response from " << Ports::convert[i] << " is " << buffer << endl;
			            // END DEBUG

			            if(converted.compare("usr_not_found") != 0){
			            	found = true;
			            	wallet += stoi(converted);
			            }
	            	}
	            	// send response back to client A
	            	if(found) 
	            	{
	            		return_msg = "successed";
	            		return_msg += " ";
	            		return_msg += to_string(wallet);
	            		return_msg += " ";

	            	} else {
	            		return_msg = "invalid_usr";
	            		return_msg += " ";
	            	}

	            	// DEBUG MSG
	            	// cout << "DEBUG: The return_msg is " << return_msg << endl;
	            	// END DEBUG

            		send(newfd, (const char*) return_msg.c_str(), strlen(return_msg.c_str()), 0);
            		cout << "The main server sent the current balance to client " << client_name << endl;
            	break;

            	//STATS()
            	case 2:
            		found = false;
	            	cout << "The main server received statistics request for " << requests[0] << endl;
	            	for (int i=0; i<3; ++i)
	            	{
	            		// setup backend port number
	            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
	            		backend_addr.sin_family = AF_INET;
						backend_addr.sin_addr.s_addr = INADDR_ANY;
						memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

						// DEBUG MSG
						// cout << "DEBUG: the received buffer is " << buffer << endl;
						// END DEBUG

						// build the backend request
						backend_msg = "check_wallet_muted";
						backend_msg += " ";
						backend_msg += requests[0];
						backend_msg += " ";

						// DEBUG MSG
						// cout << "DEBUG: backend_msg is " << backend_msg << endl;
						// END DEBUG

						// contact backend server
	            		sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
			            bzero(buffer, sizeof(buffer));
			            recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
			            converted = buffer;

			            // DEBUG MSG
			            // cout << "DEBUG: The response from " << Ports::convert[i] << " is " << buffer << endl;
			            // END DEBUG

			            if(converted.compare("usr_not_found") != 0){
			            	found = true;
			            }
	            	}

	            	// gather all the transaction statistics
	            	if(found) 
	            	{
	            		map<string, vector<int> > stats;
	            		string stats_name;
	            		for (int i=0; i<3; ++i)
		            	{
		            		// setup backend port number
		            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
		            		backend_addr.sin_family = AF_INET;
							backend_addr.sin_addr.s_addr = INADDR_ANY;
							memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

							// DEBUG MSG
							// cout << "DEBUG: the received buffer is " << buffer << endl;
							// END DEBUG

							// build the backend request
							backend_msg = "stats";
							backend_msg += " ";
							backend_msg += requests[0];
							backend_msg += " ";

							// DEBUG MSG
							// cout << "DEBUG: backend_msg is " << backend_msg << endl;
							// END DEBUG

							// contact backend server
							sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
				            converted = "";

				            // looping over UDP datagrams
				            while(1)
				            {
				            	bzero(buffer, sizeof(buffer));
				            	recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
				            	converted = buffer;

				            	if(converted.compare("EOF") == 0) break;
				         		else {
				            		vector<int> stats_vec;

				            		// convert buffer into vector
				            		// receiving format is name+trans_number+trans_amount
				            		token = strtok(buffer, " ");
				            		stats_name = token;
				            		token = strtok(NULL, " ");

							        while(token != NULL)
							        {
							            stats_vec.push_back(stoi(token));
							            token = strtok(NULL, " ");
							        }

							        // DEBUG MSG
							        // cout << "DEBUG: name is " << stats_name << endl;
							        // cout << "DEBUG: states_vec is " << stats_vec[0] << " " << stats_vec[1] << endl;
							        // END DEBUG

							        // if we already have the statistics about this name
							        if(stats.find(stats_name) != stats.end())
							        {
							        	stats[stats_name][0] += stats_vec[0];
							        	stats[stats_name][1] += stats_vec[1];
							        }
					            	else stats[stats_name] = stats_vec;
				            	}
				            }
		            	}

					    multimap<int, string, greater<int> > multi;
					    for (auto& it : stats) {
					        multi.insert(pair<int, string>(it.second[0], it.first));
					    }
					  
		            	return_msg = "successed";
		            	return_msg += " ";
		            	for(auto &v: multi){
		            		return_msg += v.second;
		            		return_msg += " ";
		            		return_msg += to_string(stats[v.second][0]);
		            		return_msg += " ";
		            		return_msg += to_string(stats[v.second][1]);
		            		return_msg += " ";
	            			// DEBUG MSG
            				// cout << "DEBUG: " << v.second << " has count: " << stats[v.second][0] << " net: " << stats[v.second][1] << endl;
            				// END DEBUG
	            		}

	            		// DEBUG MSG
	            		// cout << "DEBUG: stats response is " << return_msg << endl;
	            		// END DEBUG

	            	} else {
	            		return_msg = "user_not_found";
	            	}

	            	// DEBUG MSG
	            	// cout << "DEBUG: The return_msg is " << return_msg << endl;
	            	// END DEBUG

            		send(newfd, (const char*) return_msg.c_str(), strlen(return_msg.c_str()), 0);
            		cout << "The main server sent the statistic result to client " << client_name << endl;

            	break;

            	// TRANSACTION()
            	// code of shame.... but i just don't have that energy to figure out all the reference/pointer thing
            	case 3:
            		invalid_receiver = true;
            		invalid_sender = true;
	            	wallet = 1000;
	            	cout << "The main server received from " << requests[0] \
	            	<< " to transfer  " << requests[2] << " coins to " << requests[1] << \
	            	" using TCP over port " << ntohs(client_addr.sin_port) << endl;

	            	// first we check balance and check if sender exists
	            	for (int i=0; i<3; ++i)
	            	{
	            		// setup backend port number
	            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
	            		backend_addr.sin_family = AF_INET;
						backend_addr.sin_addr.s_addr = INADDR_ANY;
						memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

						// build the backend request, mute the response
						backend_msg = "check_wallet_muted";
						backend_msg += " ";
						backend_msg += requests[0];
						backend_msg += " ";

						// contact backend server
	            		sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
			            bzero(buffer, sizeof(buffer));
			            recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
			            converted = buffer;

			            if(converted.compare("usr_not_found") != 0){
			            	invalid_sender = false;
			            	wallet += stoi(converted);
			            }
	            	}

            		// check if receiver exists
	            	for (int i=0; i<3; ++i)
	            	{
	            		// setup backend port number
	            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
	            		backend_addr.sin_family = AF_INET;
						backend_addr.sin_addr.s_addr = INADDR_ANY;
						memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

						// build the backend request, mute the response
						backend_msg = "check_wallet_muted_serial";
						backend_msg += " ";
						// now check the receiver
						backend_msg += requests[1];
						backend_msg += " ";

						// contact backend server
	            		sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
			            bzero(buffer, sizeof(buffer));
			            recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
			            converted = buffer;

			            if(converted.compare("usr_not_found") != 0){
			            	invalid_receiver = false;
			            	// set the last transaction number
			            	last_serial = max(last_serial, stoi(converted));

			            	// DEBUG MSG
			            	// cout << "DEBUG: last_serial is " << last_serial << endl;
			            	// END DEBUG
			            }
	            	}

	            	if(invalid_sender) {
	            		return_msg = "invalid_sender";
	            		return_msg += " ";

	            		// DEBUG MSG
	            		// cout << "DEBUG: sender is invalid" << endl;
	            		// END DEBUG
	            	}

	            	if(invalid_receiver)
	            	{
	            		// DEBUG MSG
	            		// cout << "DEBUG: receiver is invalid" << endl;
	            		// END DEBUG

	            		if(invalid_sender)
	            		{
	            			return_msg = "both_invalid";
	            			return_msg += " ";

	            		} else {
	            			return_msg = "invalid_receiver";
	            			return_msg += " ";
	            		}
	            	}

	            	// if both exists and the sender has more money than transfer amount
	            	if(!invalid_receiver && !invalid_sender)
	            	{
	            		if(wallet >= stoi(requests[2]))
	            		{
	            			// update last_serial
		            		last_serial += 1;

		            		// setup backend port number, random select a server
		            		srand (time(0));
		            		rand_pick = rand() % 3;
		            		backend_addr.sin_port = htons(Ports::backend_ports[rand_pick]);
		            		backend_addr.sin_family = AF_INET;
							backend_addr.sin_addr.s_addr = INADDR_ANY;
							memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

							// build the backend request
							backend_msg = "make_transaction";
							backend_msg += " ";
							backend_msg += requests[0];
							backend_msg += " ";
							backend_msg += requests[1];
							backend_msg += " ";
							backend_msg += requests[2];
							backend_msg += " ";
							backend_msg += to_string(last_serial);

							// contact backend server
							cout << "The main server sent a request to server " << Ports::convert[rand_pick] << endl;
		            		sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
				            bzero(buffer, sizeof(buffer));
				            recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
				            cout << "The main server received the feedback from server " << Ports::convert[rand_pick]\
				            << " using UDP over port " << Ports::backend_ports[rand_pick] << endl;

				            // construct the response message to client
		            		return_msg = "successed";
		            		return_msg += " ";
		            		return_msg += to_string(wallet - stoi(requests[2]));
		            		return_msg += " ";
	            		} else {
	            			return_msg = "insufficient_balance";
	            			return_msg += " ";
	            			return_msg += to_string(wallet);
	            			return_msg += " ";
	            		}
	            	}
	            	send(newfd, (const char*) return_msg.c_str(), strlen(return_msg.c_str()), 0);
            		cout << "The main server sent the result of the transaction to client " << client_name << endl;
            	break;

            	// TXLIST()
            	case 4:
            		cout << "The main server received from " << client_name << " for transaction records" << endl;
            		map<int, string> transactions;
            		int serial_number;

            		// contact all backend servers
            		for(int i=0; i<3; ++i)
            		{
            			// setup backend port number
	            		backend_addr.sin_port = htons(Ports::backend_ports[i]);
	            		backend_addr.sin_family = AF_INET;
						backend_addr.sin_addr.s_addr = INADDR_ANY;
						memset(backend_addr.sin_zero, '\0', sizeof backend_addr.sin_zero);

						// build the backend request, mute the response
						backend_msg = "TXLIST";
						backend_msg += " ";

						// contact backend server
						sendto(backend_fd, (char*) backend_msg.c_str(), strlen(backend_msg.c_str()), 0, (const struct sockaddr*)&backend_addr, sizeof(backend_addr));
			            converted = "";

			            // looping over UDP datagrams
			            while(1)
			            {
			            	bzero(buffer, sizeof(buffer));
			            	recvfrom(servers_fd, buffer, sizeof(buffer), 0, NULL, &addrlen);
			            	converted = buffer;

			            	if(converted.compare("EOF") == 0)
			            	{
			            		break;
			            	} else {
					            serial_number = stoi(strtok(buffer, " "));
				            	transactions[serial_number] = converted;
			            	}
			            }
            		}

            		// create alichain.txt, overwrite the old file
            		ofstream out;
            		out.open("./data/alichain.txt", ios::trunc);
            		for (map<int, string>::iterator i = transactions.begin(); i != transactions.end(); i++)
					{
					    out << i->second << "\n";
					}

            		// send back and log the result
            		return_msg = "successed";
            		return_msg += " ";
            		send(newfd, (const char*) return_msg.c_str(), strlen(return_msg.c_str()), 0);
            		cout << "Transaction records ready" << endl;
            	break;
            }
            close(newfd);
        }
	}
	return 0;
}

