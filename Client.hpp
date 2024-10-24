#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
private:
    int fd;
    std::string nickname;
    std::string username;
    std::string realname;
    std::string hostname;
    bool authenticated;
    bool registered;

public:
    Client(int fd);
    Client();  // Add default constructor
    Client(int fd, const std::string& hostname);  // Constructor with fd and hostname
    
    int getFd() const;
    void setNickname(const std::string& nick) { nickname = nick; }
    void setUsername(const std::string& user) { username = user; }
    void setRealname(const std::string& real) { realname = real; }
    void setAuthenticated(bool auth) { authenticated = auth; }
    void setRegistered(bool reg) { registered = reg; }
    std::string getNickname() const { return nickname; }
    std::string getUsername() const { return username; }
    bool isAuthenticated() const { return authenticated; }
    bool isRegistered() const { return registered; }
    void send(const std::string& message);
    
    void setHostname(const std::string& hostname);
    std::string getHostname() const;
};

#endif