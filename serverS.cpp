#include "udp_connect.h"
#include "file_reader.h"
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using namespace std;

int main() {
    string path = "single.txt";
    FileReader fr(path);
    unordered_map<string, string> singleRoomStatus = fr.readFile(false);
    const unordered_map<string, string> singleRoomInfo = singleRoomStatus;
    string serverCode = "S";
    const string SERVER_S_PORT = "41282";
    const string SERVER_M_PORT = "44282";

    cout << "The Server " << serverCode << " is up and running using UDP on port " << SERVER_S_PORT << endl;

    thread udpSThread(udpServer, serverCode, ref(singleRoomStatus));
    thread udpCThread(udpClient, singleRoomInfo, SERVER_M_PORT, serverCode);

    udpSThread.join();
    udpCThread.join();

    return 0;
}

