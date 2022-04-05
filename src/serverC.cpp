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
#include <map>
#include <fstream>
#include <algorithm>
#include "back_servers.h"
#include "constants.h"

using namespace std;

int main(int argc, char *argv[])
{
    int mysock, server_sock;
    char buffer[1024] = {0};
    struct sockaddr_in myaddr;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    string response;
    bool found;

    // read and parse my log file
    int serial, amount, net_transaction;
    int last_serial = 0;
    map<string, map<string, vector<int> > > sending;
    map<string, map<string, vector<int> > > receiving;
    string sender, receiver;

    // a nested scope so that RAII can close the file once it finishes
    if(true)
    {
        ifstream infile("./data/block3.txt");
        while (infile >> serial >> sender >> receiver >> amount)
        {
            sending[sender][receiver].push_back(amount);
            receiving[receiver][sender].push_back(amount);
            last_serial = max(serial, last_serial);
        }
    }

    // create my socket
    mysock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(Ports::serverC);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(mysock, (struct sockaddr*)&myaddr, sizeof(myaddr));

    // create server socket
    server_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Ports::serverM_S);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // send boot-up message
    cout << "The ServerC is up and running using UDP on port " << ntohs(myaddr.sin_port) << endl;

    while(1)
    {
        // wait to recv message from server M
        bzero(buffer, sizeof(buffer));
        recvfrom(mysock, (char*)buffer, 1024, 0, NULL, &addrlen);

        // parse the requests from server
        // requests is a vector <request_type, **args>
        char* token;
        vector<string> requests;
        token = strtok(buffer, " ");
        while(token != NULL)
        {
            requests.push_back(token);
            token = strtok(NULL, " ");
        }
        response = "";

        // log the initial response
        if(requests[0].compare("check_wallet") == 0 || requests[0].compare("make_transaction") == 0)
        {
            cout << "The ServerC received a request from the Main Server." << endl;
        }

        // DEBUG MSG
        // cout << "DEBUG: requests[0] is " << requests[0] << endl;
        // cout << "DEBUG: requests[1] is " << requests[1] << endl;
        // END DEBUG

        // execute the request
        if(requests[0].compare("check_wallet") == 0 || requests[0].compare("check_wallet_muted") == 0\
           || requests[0].compare("check_wallet_muted_serial") == 0)
        {
            found = false;
            net_transaction = 0;

            // if the usr is in sender
            if(sending.find(requests[1]) != sending.end())
            {
                // DEBUG MSG
                // cout << "DEBUG: found " << requests[1] << " in sending" << endl;
                // END DEBUG

                found = true;
                for(auto &s: sending.at(requests[1]))
                {
                    for(int t: s.second)
                    {
                        net_transaction -= t;
                    }
                }
            }

            if(receiving.find(requests[1]) != receiving.end())
            {
                // DEBUG MSG
                // cout << "DEBUG: found " << requests[1] << " in receiving" << endl;
                // END DEBUG

                found = true;
                for(auto &s: receiving.at(requests[1]))
                {
                    for(int t: s.second)
                    {
                        net_transaction += t;
                    }
                }
            }

            if(found) 
            {
                if(requests[0].compare("check_wallet_muted_serial") != 0)
                {
                    response = to_string(net_transaction);

                } else {
                    response = to_string(last_serial);
                }
            } else {
                response = "usr_not_found";
            }

        } else if(requests[0].compare("make_transaction") == 0)
        {
            last_serial = stoi(requests[4]);
            sending[requests[1]][requests[2]].push_back(stoi(requests[3]));
            receiving[requests[2]][requests[1]].push_back(stoi(requests[3]));
            ofstream out;
            out.open("./data/block3.txt", ios::app);
            string to_write = "\n";
            to_write += requests[4];
            to_write += " ";
            to_write += requests[1];
            to_write += " ";
            to_write += requests[2];
            to_write += " ";
            to_write += requests[3];
            out << to_write;
            out.close();

            // DEBUG MSG
            // cout << "DEBUG: wrote " << to_write << " into block3" << endl;
            // END DEBUG
        } else if (requests[0].compare("TXLIST") == 0) {
            ifstream infile("./data/block3.txt");
            while (infile >> serial >> sender >> receiver >> amount)
            {
                response = to_string(serial);
                response += " ";
                response += sender;
                response += " ";
                response += receiver;
                response += " ";
                response += to_string(amount);
                response += " ";
                sendto(server_sock, (char*)response.c_str(), strlen(response.c_str()), 0, (const struct sockaddr*)&server_addr, sizeof(server_addr));
            }

            response = "EOF";
        }
        
        // send back the result
        sendto(server_sock, (char*)response.c_str(), strlen(response.c_str()), 0, (const struct sockaddr*)&server_addr, sizeof(server_addr));
        if(requests[0].compare("check_wallet") != 0 || requests[0].compare("make_transaction") == 0)
        {
            cout << "The ServerB finished sending the response to the Main Server." << endl;
        }
    }

    return 0;
}