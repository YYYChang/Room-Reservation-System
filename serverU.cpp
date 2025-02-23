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
    string path = "suite.txt";
    FileReader fr(path);
    unordered_map<string, string> suiteStatus = fr.readFile(false);
    const unordered_map<string, string> suiteInfo = suiteStatus;
    string serverCode = "U";
    const string SERVER_U_PORT = "43282";
    const string SERVER_M_PORT = "44282";
    
    cout << "The Server " << serverCode << " is up and running using UDP on port " << SERVER_U_PORT << endl;

    thread udpSThread(udpServer, serverCode, ref(suiteStatus));
    thread udpCThread(udpClient, suiteInfo, SERVER_M_PORT, serverCode);

    udpSThread.join();
    udpCThread.join();

    return 0;
}