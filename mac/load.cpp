#include "load.h"

//read out a file, given the file name (why tf do I have to write a function for this?)
std::string readFile(std::string filePath) {
    std::string content;
    std::ifstream srcStream(filePath, std::ios::in);
    if (!srcStream.is_open()) {
        std::cerr << "Could not open file " << filePath << std::endl;
        return "";
    }
    std::string line = "";
    while (!srcStream.eof()) {
        getline(srcStream, line);
        content.append(line + "\n");
    }
    srcStream.close();
    return content;
}

