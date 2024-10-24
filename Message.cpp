#include "Message.hpp"
#include <sstream>

Message::Message(const std::string& rawMessage) {
    std::istringstream iss(rawMessage);
    std::string token;

    if (rawMessage[0] == ':') {
        iss >> prefix;
        prefix = prefix.substr(1);
    }

    iss >> command;

    while (iss >> token) {
        if (token[0] == ':') {
            std::string trailing = token.substr(1);
            std::getline(iss, token);
            trailing += token;
            params.push_back(trailing);
            break;
        }
        params.push_back(token);
    }
}