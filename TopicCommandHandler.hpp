#ifndef TOPIC_COMMAND_HANDLER_HPP
#define TOPIC_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class TopicCommandHandler : public CommandHandler {
public:
    TopicCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif