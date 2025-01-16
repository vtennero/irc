#ifndef PRIVMSG_COMMAND_HANDLER_HPP
#define PRIVMSG_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

// class PrivmsgCommandHandler : public CommandHandler {
// public:
// 	PrivmsgCommandHandler(Server& server);
// 	virtual void handle(Client& client, const Message& message);

// private:
// 	bool validateMessageParams(const Message& message, string& target, string& content);
// 	void sendPrivateMessage(Client& sender, const string& targetNick, const string& message);
// 	void sendChannelMessage(Client& sender, const string& channelName, const string& message);
// 	string formatMessage(const Client& sender, const string& target, const string& message);
// 	string trimMessage(const string& message);
// 	bool validateTarget(Client& sender, const string& targetNick, Client** target);
// };

class PrivmsgCommandHandler : public CommandHandler {
public:
    explicit PrivmsgCommandHandler(Server& server);
    virtual void handle(Client& sender, const Message& message);
    void sendPrivateMessage(Client& sender, const std::string& targetNick, const std::string& message);
    void sendChannelMessage(Client& sender, const std::string& channelName, const std::string& message);

private:
    // Validation helpers
    bool validateMessageParams(const Message& message);
    std::string trimMessage(const std::string& message);
    bool validateTarget(Client& sender, Client* target, const std::string& targetNick);

    // Logging helpers
    void logMessageAttempt(const std::string& senderNick, const std::string& target, const std::string& content);
    void logFormattedMessage(const std::string& formattedMessage);

    // Message formatting
    std::string formatMessage(const Client& sender, const std::string& target, const std::string& message);
    std::string formatBasicPrivmsg(const Client& sender, const std::string& target, const std::string& content);
};


#endif
