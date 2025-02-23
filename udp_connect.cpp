#include "udp_connect.h"
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

/* refer to Beej’s Guide to Network Programming and Linkedin Learning: 
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void udpServer(const string &servertype, unordered_map<string, string> &curStatus) {
    const char SERVER_S_PORT[] = "41282";
    const char SERVER_D_PORT[] = "42282";
    const char SERVER_U_PORT[] = "43282";

    struct addrinfo hints, *serverInfo;
    int sockfd;
    // int rv;
    int numBytes;
    socklen_t addr_len;
    struct sockaddr client;
    const int size = 1024;
    char buffer[size];
    char ipStr[size];

    // get host name and port
    const char *port;
    if (servertype == "S") {
        port = SERVER_S_PORT;
    } else if (servertype == "D") {
        port = SERVER_D_PORT;
    } else {
        port = SERVER_U_PORT;
    }

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP Datagram
    hints.ai_flags = AI_PASSIVE; // accept any connection
    
    if (getaddrinfo("127.0.0.1", port, &hints, &serverInfo) != 0) {
        perror("failed");
        exit(1);
    }

    // create a socket 
    if ((sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1) {
        perror("socket create failed");
        exit(1);
    }

    // bind the server to the socket
    if (bind(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        perror("binding failed");
        exit(1);
    }

    // keep udp server until user close
    while (true) {
        // receive availability/reservation request from server M
        addr_len = sizeof(struct sockaddr);
        numBytes = recvfrom(sockfd, ipStr, size, 0, &client, &addr_len);
        if (numBytes == -1) {
            perror("receiving step failed");
            exit(1);
        }
        ipStr[numBytes] = '\0';

        istringstream iss(ipStr);
        string roomCode, command;
        getline(iss, roomCode);
        getline(iss, command);

        cout << "The Server " << servertype << " received an ";
        if (command == "A") {
            cout << "availability ";
        } else {
            cout << "reservation ";
        }
        cout << "request from the main server." << endl;

        // reply status code to server M
        /* IA: invalid roomCode for avialaility
           IR: invalid roomCode for reservation
           E: server side error
           A: room available
           N: room not available
           S: reservation succeed
           F: reservation failed */
        auto roomData = curStatus.find(roomCode);
        if (command == "A") {
            if (roomData == curStatus.end()) {
                cout << "Not able to find the room layout." << endl;
                strcpy(buffer, "IA");
            } else if (stoi(roomData->second) > 0) {
                cout << "Room " << roomCode << " is available." << endl;
                strcpy(buffer, "A");
            } else {
                cout << "Room " << roomCode << " is not available." << endl;
                strcpy(buffer, "U");
            }
        } else {
            if (roomData == curStatus.end()) {
                cout << "Cannot make a reservation. Not able to find the room layout." << endl;
                strcpy(buffer, "IR");
            } else if (stoi(roomData->second) > 0) {
                roomData->second = to_string(stoi(roomData->second) - 1);

                cout << "Successful reservation. The count of Room ";
                cout << roomCode << " is now " << roomData->second << "." << endl;
                strcpy(buffer, "S");
            } else {
                cout << "Cannot make a reservation. Room " << roomCode << " is not available." << endl;
                strcpy(buffer, "F");
            }
        }

        if (sendto(sockfd, buffer, strlen(buffer), 0, &client, addr_len) == -1) {
            perror("sending response failed");
            exit(1);
        }

        if (string(buffer) == "S") {
            cout << "The Server " << servertype << " finished sending the response ";
            cout << "and the updated room status to the main server." << endl;
        }
        else {
            cout << "The Server " << servertype << " finished sending the response to the main server." << endl;
        }
    }

    // UDP data exchange finish free up space and close socket
    freeaddrinfo(serverInfo);
    close(sockfd);
}

/* refer to Beej’s Guide to Network Programming and Linkedin Learning: 
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void udpClient(const unordered_map<string, string> &data, const string &serverPort, const string &type) {
    struct addrinfo hints, *host;
    // int rv;
    // int numBytes;
    int sockfd;
    // const int size = 1024;
	// char buffer[size];

    // configure remote address (server)
    memset(&hints, 0, sizeof(hints));// make sure the struct is empty
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP Datagram

    if (getaddrinfo("127.0.0.1", serverPort.c_str(), &hints, &host) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // create a socket
    if ((sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol)) == -1) {
        perror("socket create failed");
        exit(1);
    }

    // convert map data to string
    stringstream ss;
    ss << type << "\n";
    for (const auto &pair : data) {
        ss << pair.first << "," << pair.second << "\n";
    }
    string dataStr = ss.str();
    // cout << dataStr << endl;
    const char *roomData = dataStr.c_str();

    // initiate UDP request, (bind in here)
    cout << "The Server " << type << " has sent the room status to the main server." << endl;
    sendto(sockfd, roomData, strlen(roomData), 0, host->ai_addr, host->ai_addrlen);
    
    // numBytes = recvfrom(sockfd, buffer, size, 0, host->ai_addr, &host->ai_addrlen);
    // buffer[numBytes] = '\0';

    // UDP data exchange finish free up space and close socket
    freeaddrinfo(host);
    close(sockfd);
}