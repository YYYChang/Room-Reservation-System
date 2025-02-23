#ifndef FILE_READER_H
#define FILE_READER_H

#include <sstream>
#include <string>
#include <unordered_map>

/* read source file and store data into map */
class FileReader {
public:
    FileReader();
    FileReader(const std::string& path);
    std::unordered_map<std::string, std::string> readFile(bool keyToLower);
    std::unordered_map<std::string, std::string> readFile(std::istringstream& iss, std::string& type);

private:
    std::string filePath;

    /* trim space at start/end of string */
    std::string trim(const std::string& str);
    void readHelper(std::basic_istream<char>& stream, std::unordered_map<std::string, std::string>& map, bool keyToLower);
};

#endif