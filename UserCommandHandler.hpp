#ifndef USER_COMMAND_HANDLER_HPP
#define USER_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class UserCommandHandler : public CommandHandler
{
	public:
		UserCommandHandler(Server& server);
		virtual void handle(Client& client, const Message& message);
		void sendWelcomeMessages(Client& client);

	private:
		bool validateRegistrationStatus(Client& client);
		bool validateMessageParameters(Client& client, const Message& message);
		void processUserInformation(Client& client, const Message& message);
		void checkAndCompleteRegistration(Client& client);

		void sendWelcomeMessage(Client& client, const string& nickname, const string& username, const string& hostname);
		void sendHostInfo(Client& client, const string& nickname);
		void sendServerCreationInfo(Client& client, const string& nickname);
		void sendServerCapabilities(Client& client, const string& nickname);
};

#endif
