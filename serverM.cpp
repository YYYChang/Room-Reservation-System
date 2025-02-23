#include "file_reader.h"
#include <atomic>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using namespace std;
unordered_map<string, string> userInfo;
unordered_map<string, string> singleRoomInfo;
unordered_map<string, string> doubleRoomInfo;
unordered_map<string, string> suiteInfo;

/* refer to Beej’s Guide to Network Programming and Linkedin Learning:
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void udpServer() {
    const char *port = "44282";
    struct addrinfo hints, *serverInfo;
    int sockfd;
    // int rv;
    int numBytes;
    socklen_t addr_len;
    struct sockaddr client;
    const int size = 1024;
    char ipStr[size];

    // get host name and port
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_DGRAM;   // UDP Datagram
    hints.ai_flags = AI_PASSIVE;      // accept any connection

    if (getaddrinfo("127.0.0.1", port, &hints, &serverInfo) != 0) {
        perror("getaddrinfo failed");
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

    // keep udp server until three set of data received close
    // piazza @169 the server will always start in the seq M > S > D > U
    int count = 0;
    // while (singleRoomInfo.size() == 0 || doubleRoomInfo.size() == 0 || suiteInfo.size() == 0) {
    while (count < 3) {
        addr_len = sizeof(struct sockaddr);
        numBytes = recvfrom(sockfd, ipStr, size, 0, &client, &addr_len);
        count++;
        if (numBytes == -1) {
            perror("receiving step failed");
            exit(1);
        }
        ipStr[numBytes] = '\0';
        istringstream inputss(ipStr);
        string type;
        FileReader fr;
        unordered_map<string, string> inputMap = fr.readFile(inputss, type);

        if (type == "S") {
            singleRoomInfo = inputMap;
        } else if (type == "D") {
            doubleRoomInfo = inputMap;
        } else {
            suiteInfo = inputMap;
        }

        cout << "The main server has received the room status from Server " << type;
        cout << " using UDP over port " << port << "." << endl;
    }

    // UDP data exchange finish free up space and close socket
    freeaddrinfo(serverInfo);
    close(sockfd);
}

/* refer to Beej’s Guide to Network Programming and Linkedin Learning:
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void udpClient(char *message, char *buffer, int bufferSize, string &roomCode) {
    const char SERVER_S_PORT[] = "41282";
    const char SERVER_D_PORT[] = "42282";
    const char SERVER_U_PORT[] = "43282";
    const char SERVER_M_UDP_PORT[] = "44282";

    struct addrinfo hints, *host;
    // int rv;
    int numBytes;
    int sockfd;
    struct sockaddr_in clientAddr;

    const char *port;
    if (message[0] == 'S') {
        port = SERVER_S_PORT;
    } else if (message[0] == 'D') {
        port = SERVER_D_PORT;
    } else {
        port = SERVER_U_PORT;
    }

    // configure remote address (server)
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_DGRAM;   // UDP Datagram

    if (getaddrinfo("127.0.0.1", port, &hints, &host) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // create a socket
    if ((sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol)) == -1) {
        perror("socket create failed");
        exit(1);
    }

    // bind the socket to the client port
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(stoi(SERVER_M_UDP_PORT));

    if (bind(sockfd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1) {
        perror("udp client socket binding failed");
        exit(EXIT_FAILURE);
    }

    // initiate UDP request, (bind in here)
    sendto(sockfd, message, strlen(message), 0, host->ai_addr, host->ai_addrlen);

    /* Get the local port number of the client socket
       refernce to how to get TCP client port number:
       project requirements, github: https://gist.github.com/listnukira/4045436, and chatgpt */
    // struct sockaddr_in udpClientAddr;
    // socklen_t addrLen = sizeof(udpClientAddr);
    // if (getsockname(sockfd, (struct sockaddr *)&udpClientAddr, (socklen_t *)&addrLen) == -1) {
    //     perror("getsockname failed");
    //     exit(1);
    // }
    // int udpClientPort = ntohs(udpClientAddr.sin_port);

    /* if udpServer response is not received within 2 sec send an time-out alarm */
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        freeaddrinfo(host);
        exit(1);
    }

    numBytes = recvfrom(sockfd, buffer, bufferSize, 0, host->ai_addr, &host->ai_addrlen);
    buffer[numBytes] = '\0';

    if (numBytes < 1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            strcpy(buffer, "E");
            printf("Timeout occurred. Server did not respond.\n");
        } else {
            perror("recvfrom failed");
        }
    } else {
        if (string(buffer) == "S") {
            cout << "The main server received the response and the updated room status from Server ";
            cout << roomCode[0] << " using UDP over port " << SERVER_M_UDP_PORT << "." << endl;

            if (roomCode[0] == 'S') {
                singleRoomInfo[roomCode] = to_string(stoi(singleRoomInfo[roomCode]) - 1);
            } else if (roomCode[0] == 'D') {
                doubleRoomInfo[roomCode] = to_string(stoi(doubleRoomInfo[roomCode]) - 1);
            } else {
                suiteInfo[roomCode] = to_string(stoi(suiteInfo[roomCode]) - 1);
            }

            cout << "The room status of Room " << roomCode << " has been updated." << endl;
        } else {
            cout << "The main server received the response from Server ";
            cout << roomCode[0] << " using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
        }
    }

    // UDP data exchange finish free up space and close socket
    freeaddrinfo(host);
    close(sockfd);
}

// verify username/password w member data
bool passwordVerification(const string &username, const string &password, string &failPart) {
    auto userData = userInfo.find(username);
    if (userData == userInfo.end()) {
        failPart = "U";
        return false;
    }

    if (userData->second != password) {
        failPart = "P";
        return false;
    }
    return true;
}

/* refer to Beej’s Guide to Network Programming and Linkedin Learning:
   Network Programming in C: Develop Reliable Client/Server Applications Course Material */
void tcpServer() {
    const char *port = "45282";
    struct addrinfo hints, *serverInfo;
    // int rv;
    int numBytes;
    char buffer[BUFSIZ];
    struct sockaddr address;
    socklen_t addr_len = sizeof(struct sockaddr);
    fd_set main_fd;

    // storage all connections info
    const int ipv4_size = 32;
    char hostname[ipv4_size];
    const int max_connect = 10;
    char connection[max_connect][ipv4_size];

    // storage all connections identify and name
    const int NULL_CODE = -1, GUEST_CODE = 0, MEMBER_CODE = 1;
    int clientIdentity[max_connect + 1];
    string clientName[max_connect + 1];
    for (int i = 0; i <= max_connect; i++) {
        clientIdentity[i] = NULL_CODE;
        clientName[i] = "";
    }

    // configure TCP server
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;      // accept any connection

    if (getaddrinfo("127.0.0.1", port, &hints, &serverInfo) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // create parent socket
    int parentfd;
    if ((parentfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1) {
        perror("parent socket create failed");
        exit(1);
    }

    // bind the server info with the socket
    if (bind(parentfd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        perror("binding failed");
        exit(1);
    }

    // parent socket succeess start listening to incoming connection
    if (listen(parentfd, max_connect) == -1) {
        perror("listening failed");
        exit(1);
    }

    FD_ZERO(&main_fd);          // clear the set ahead of time
    FD_SET(parentfd, &main_fd); // set parent descriptor */

    // loop tcp server
    fd_set read_fds;
    while (true) {
        read_fds = main_fd;

        // select incoming connections
        if (select(max_connect + 1, &read_fds, NULL, NULL, 0) == -1) {
            perror("select failed");
            exit(1);
        }

        // proccess connections from # 1 to max_connect
        for (int fd = 1; fd <= max_connect; fd++) {
            // find fd waiting for connect
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == parentfd) { // parent socket accepting waiting query
                    // accept connection and create child socket
                    int childfd = accept(parentfd, &address, &addr_len);
                    if (childfd == -1) {
                        perror("accept child failed");
                        exit(1);
                    }
                    // get tcp client name info and sore to coresponding list
                    if (getnameinfo(&address, addr_len, hostname, ipv4_size, 0, 0, NI_NUMERICHOST) == -1) {
                        perror("getnameinfo failed");

                        // clear fd and close connection
                        FD_CLR(fd, &main_fd);
                        close(fd);
                    }

                    strcpy(connection[childfd], hostname);
                    FD_SET(childfd, &main_fd);
                } else { // existing
                    // userInfo authentication:
                    if (clientIdentity[fd] == NULL_CODE) {
                        numBytes = recv(fd, buffer, BUFSIZ, 0);
                        if (numBytes <= 0) {
                            // cout << "authentication recv step error!" << endl;
                            // clear fd and close connection
                            FD_CLR(fd, &main_fd);
                            close(fd);

                            // clear current client identity
                            clientIdentity[fd] = NULL_CODE;
                            clientName[fd] = "";
                        }
                        buffer[numBytes] = '\0';

                        istringstream iss(buffer);
                        string username, password;
                        string failPart;
                        string returnMsg;

                        getline(iss, username);
                        getline(iss, password);

                        /* update status message on screen
                           piazza @136, print encrypted username*/
                        if (password.length() == 0) {
                            cout << "The main server received the guest request for " << username;
                            cout << " using TCP over port " << port << ".";
                            cout << " The main server accepts " << username << " as a guest." << endl;

                            clientIdentity[fd] = GUEST_CODE;
                            cout << "The main server sent the guest response to the client." << endl;
                        } else {
                            cout << "The main server received the authentication for " << username;
                            cout << " using TCP over port " << port << "." << endl;

                            cout << "The main server sent the authentication result to the client." << endl;
                            if (!passwordVerification(username, password, failPart)) {
                                if (failPart == "U") {
                                    returnMsg = "failU";
                                } else {
                                    returnMsg = "failP";
                                }

                                send(fd, returnMsg.c_str(), returnMsg.length(), 0);
                                continue;
                            }
                            clientIdentity[fd] = MEMBER_CODE;
                        }
                        returnMsg = "succeed";
                        clientName[fd] = username;

                        // feedback authentication status
                        send(fd, returnMsg.c_str(), returnMsg.length(), 0);

                        continue;
                    }

                    // normal user request:
                    numBytes = recv(fd, buffer, BUFSIZ, 0);
                    if (numBytes < 1) {
                        // cout << "normal recv step error!" << endl;
                        // clear fd and close connection
                        FD_CLR(fd, &main_fd);
                        close(fd);

                        // clear current client identity
                        clientIdentity[fd] = NULL_CODE;
                        clientName[fd] = "";
                    } else {
                        buffer[numBytes] = '\0';

                        // cpy a new string
                        char bufferCp[strlen(buffer) + 1];
                        strcpy(bufferCp, buffer);

                        istringstream iss(buffer);
                        string roomCode, command;
                        getline(iss, roomCode);
                        getline(iss, command);

                        cout << "the main server has received the ";
                        if (command == "A") {
                            cout << "availability ";
                        } else {
                            cout << "reservation ";
                        }
                        cout << "request on Room " << roomCode << " from " << clientName[fd];
                        cout << " using TCP over port " << port << "." << endl;

                        /* PD: permission denied, due to identity
                           IA: invalid roomCode for avialaility
                           IR: invalid roomCode for reservation
                           E: server side error
                           A: room available
                           N: room not available
                           S: reservation succeed
                           F: reservation failed */
                        const int udpCbufferSize = 1024;
                        char udpCbuffer[udpCbufferSize];
                        char returnMsg[udpCbufferSize];
                        string returnStr;
                        if (roomCode.empty() || (roomCode[0] != 'S' && roomCode[0] != 'D' && roomCode[0] != 'U')) {
                            if (command == "A") {
                                strcpy(returnMsg, "IA");
                            } else {
                                strcpy(returnMsg, "IR");
                            }
                        } else if (clientIdentity[fd] == GUEST_CODE && command == "R") {
                            cout << clientName[fd] << " cannot make a reservation." << endl;
                            strcpy(returnMsg, "PD");
                        } else {
                            cout << "The main server sent a request to Server " << roomCode[0] << "." << endl;
                            udpClient(bufferCp, udpCbuffer, udpCbufferSize, roomCode);

                            if (strlen(udpCbuffer) < 1) {
                                strcpy(returnMsg, "E");
                            } else {
                                strcpy(returnMsg, udpCbuffer);
                                returnStr = string(returnMsg);
                            }
                        }

                        // send status check result back to tcp client
                        send(fd, returnMsg, strlen(returnMsg), 0);
                        returnStr = string(returnMsg);

                        if (returnStr == "E" || returnStr == "PD" || returnStr == "IA" || returnStr == "IR") {
                            cout << "The main server sent the error message to the client." << endl;
                        }

                        cout << "The main server sent the ";
                        if (command == "R") {
                            cout << "reservation result";
                        } else {
                            cout << "availability information";
                        }

                        cout << " to the client." << endl;
                    }
                }
            }
        }
    }

    freeaddrinfo(serverInfo);
    close(parentfd);
}

// user data
int main() {
    string path = "member.txt";
    FileReader fr(path);
    userInfo = fr.readFile(true);

    cout << "The main server is up and running." << endl;
    thread tcpThread(tcpServer);
    thread udpSThread(udpServer);
    // thread udpCThread(udpClient);

    tcpThread.join();
    udpSThread.join();
    // udpCThread.join();

    return 0;
}