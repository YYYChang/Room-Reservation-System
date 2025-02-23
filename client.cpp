#include <algorithm>
#include <cctype>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

using namespace std;

// ask for user sign-in info input
void requestUserInfo(string &username, string &password) {
    cout << "Please enter the username: ";
    getline(cin, username);
    for (size_t i = 0; i < username.length(); i++) {
        username[i] = tolower(username[i]);
    }

    // piazza @137
    cout << "Please enter the password (Press “Enter” to skip for guest): ";
    getline(cin, password);
}

// enable user to reserve a room or query status, keep looping until user insert correct cmd
void requestUserCmd(string &roomCode, string &command) {
    cout << "Please enter the room code: ";
    cin >> roomCode;

    // piazza @152, keep asking for cmd until it is valid
    bool valid = false;
    while (!valid) {
        cout << "Would you like to search for the availability or make a reservation? ";
        cout << "(Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): ";
        cin >> command;

        if (command != "Availability" && command != "Reservation") {
            cout << "Invalid choice: Please input Availability/Reservation" << endl;
            continue;
        }
        valid = true;
    }
}

// user input username/password encrpytion 
string encryptUserInfo(string str) {
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c >= 'a' && c <= 'z') {
            str[i] = (c - 'a' + 3) % 26 + 'a';
        } else if (c >= 'A' && c <= 'Z') {
            str[i] = (c - 'A' + 3) % 26 + 'A';
        }

        else if ('0' <= c && '9' >= c) {
            str[i] = (c - '0' + 3) % 10 + '0';
        }
    }

    return str;
}

/* refer to Beej’s Guide to Network Programming and Linkedin Learning: 
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void tcpClient() {
    const char *port = "45282";
    int sockfd, numBytes;
    char buffer[BUFSIZ];
    struct addrinfo hints, *serverInfo; 
    int rv;

    // get server name and port
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    
    if ((rv = getaddrinfo("127.0.0.1", port, &hints, &serverInfo)) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // create a socket
    if ((sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1) {
        perror("socket create failed");
        exit(1);
    }

    // connect to the server socket
    if (connect(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        close(sockfd);
        perror("connect failed");
        exit(1);
    }

    /* Get the local port number of the client socket
       refernce of how to get TCP client port number:
       project requirements, github: https://gist.github.com/listnukira/4045436, and chatgpt */
    struct sockaddr_in tcpClientAddr;
    socklen_t addrLen = sizeof(tcpClientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&tcpClientAddr, (socklen_t *)&addrLen) == -1) {
        perror("getsockname failed");
    }
    int tcpClientPort = ntohs(tcpClientAddr.sin_port);

    /* create and send sign-in info to TCP server
       piazza @206, wrong password should keep asking for user input */
    bool authentication = false;
    string username;
    string password;
    while (!authentication) {
        requestUserInfo(username, password);

        string signinInfo = encryptUserInfo(username) + "\n";
        signinInfo += encryptUserInfo(password);

        send(sockfd, signinInfo.c_str(), signinInfo.length(), 0);
        if (password.length() == 0) {
            cout << username << " sent a guest request to the main server using TCP over port " << tcpClientPort << "." << endl;
        } else {
            cout << username << " sent an authentication request to the main server." << endl;
        }

        // authentication status feedback by server
        numBytes = recv(sockfd, buffer, BUFSIZ, 0);
        if (numBytes < 1) {
            cout << "TCP connection closed";
            break;
        }
        buffer[numBytes] = '\0';

        string authFeedback(buffer);
        if (authFeedback == "succeed") {
            cout << "Welcome ";
            if (password.length() > 0) {
                cout << "member ";
            } else {
                cout << "guest ";
            }
            cout << username << "!" << endl;
            authentication = true;
        } else if (authFeedback == "failU") {
            cout << "Failed login: Username does not exist." << endl;
        } else {
            cout << "Failed login: Password does not match." << endl;
        }
    }

    // loop and interact with the tcp server
    while (true) {
        // user input
        string roomCode;
        string command;
        requestUserCmd(roomCode, command);
        string usercmd = roomCode + "\n";

        cout << username << " sent an ";
        if (command == "Availability") {
            usercmd += "A\n";
            cout << "availability ";
        } else {
            usercmd += "R\n";
            cout << "reservation ";
        }
        cout << "request to the main server." << endl;
        send(sockfd, usercmd.c_str(), usercmd.length(), 0);

        numBytes = recv(sockfd, buffer, BUFSIZ, 0);
        if (numBytes < 1) {
            cout << "TCP connection closed";
            break;
        }
        buffer[numBytes] = '\0';

        /* PD: permission denied, due to identity
           IA: invalid roomCode for avialaility
           IR: invalid roomCode for reservation
           E: server side error
           A: room available
           N: room not available
           S: reservation succeed
           F: reservation failed */
        string bufferStr = string(buffer);
        if (bufferStr == "E") {
            cout << "Server not accessible" << endl;
        } else if (bufferStr != "PD") {
            cout << "The client received the response from the main server using TCP over port " << tcpClientPort << "." << endl;
            if (bufferStr == "IA") {
                cout << "Not able to find the room layout." << endl;
            } else if (bufferStr == "IR") {
                cout << "Oops! Not able to find the room." << endl;
            } else if (bufferStr == "A") {
                cout << "The requested room is available." << endl;
            } else if (bufferStr == "U") {
                cout << "The requested room is not available." << endl;
            } else if (bufferStr == "S") {
                cout << "Congratulation! The reservation for Room " << roomCode << " has been made." << endl;
            } else if (bufferStr == "F") {
                cout << "Sorry! The requested room is not available." << endl;
            }
            cout << "\n-----Start a new request-----" << endl;
        } else {
            cout << "Permission denied: Guest cannot make a reservation." << endl;
        }
    }

    // TCP data exchange finish free up space and close socket
    freeaddrinfo(serverInfo);
    close(sockfd);
}

int main() {
    cout << "Client is up and running." << endl;
    thread tcpThread(tcpClient);

    tcpThread.join();

    return 0;
}
