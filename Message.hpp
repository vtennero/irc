#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message {
private:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;

public:
    Message(const std::string& rawMessage);
    std::string getCommand() const { return command; }
    const std::vector<std::string>& getParams() const { return params; }
};

#endif