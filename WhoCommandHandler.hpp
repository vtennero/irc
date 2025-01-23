#ifndef WHO_COMMAND_HANDLER_HPP
#define WHO_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class WhoCommandHandler : public CommandHandler {

public:
	WhoCommandHandler(Server& server);
	virtual void handle(Client& client, const Message& message);
};

#endif
