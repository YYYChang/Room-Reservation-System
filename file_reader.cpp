#include "file_reader.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

FileReader::FileReader() {}

FileReader::FileReader(const string& path) {
    filePath = path;
}

unordered_map<string, string> FileReader::readFile(bool keyToLower) {
    unordered_map<string, string> content;
    ifstream inputFile(filePath);

    if (!inputFile) {
        cout << "Failed to open the file \"" << filePath << "\"" << endl;
        return content;
    }

    readHelper(inputFile, content, keyToLower);

    inputFile.close();
    return content;
}

unordered_map<string, string> FileReader::readFile(istringstream& iss, string& type) {
    getline(iss, type);
    unordered_map<string, string> content;
    readHelper(iss, content, false);

    return content;
}

void FileReader::readHelper(basic_istream<char>& stream, unordered_map<string, string>& map, bool keyToLower) {
    string line;
    while (getline(stream, line)) {
        stringstream strStream(line);
        string key;
        string value;
        if (getline(strStream, key, ',')) {
            getline(strStream, value);
        }

        if (keyToLower) {
            for (size_t i = 0; i < key.size(); i++) {
                key[i] = tolower(key[i]);
            }
        }

        map[trim(key)] = trim(value);
    }
}

string FileReader::trim(const string& str) {
    size_t start = str.find_first_not_of(" ");
    if (start == string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(" ");
    return str.substr(start, end - start + 1);
}