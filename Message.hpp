#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message {
private:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::string rawMessage;  // Store the original raw message
    std::string token;  // Add token field for PING/PONG messages

public:
    Message(const std::string& rawMessage);
    std::string getCommand() const { return command; }
    std::string getPrefix() const { return prefix; }
    const std::vector<std::string>& getParams() const { return params; }
    const std::string& getRawMessage() const { return rawMessage; }
    const std::string& getToken() const { return token; }  // Add getter for token

private:
    void parseToken();  // Add method to extract token

};

#endif
