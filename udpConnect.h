#ifndef UDPCONNECT_H
#define UDPCONNECT_H

#include <unordered_map>
#include <string>

void udpServer(const std::string& serverPort, std::unordered_map<std::string, std::string>& curStatus);
void udpClient(const std::unordered_map<std::string, std::string>& data, const std::string& serverPort, const std::string& type);

#endif