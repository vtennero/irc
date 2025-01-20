#include "ModeCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstdlib>

static bool isChannel(const string& channelName) {
		return !channelName.empty() && channelName[0] == '#';
}

static void parseModeCmd(const string& cmd, Channel* channel, Client& client, Server* server, vector<string>&modeArgs)
{
	cout << "running parseModeCmd" << endl;
	int modeflag = 1;
	for (size_t i = 0; i < cmd.length(); ++i) {
		cout << "parsing " << cmd[i] << endl;
		if (cmd[i] == '+') modeflag = 1;
		else if (cmd[i] == '-') modeflag = 0;
		//handle individual commands
		else if (i == 'i' || i == 't') {
			channel->setMode(cmd[i], modeflag);
			client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +k " + modeArgs[0] + "\r\n");
		} else if (i == 'k' || i == 'o' || i == 'l') {
			if (modeflag == 1) {
					if (modeArgs.size() == 0) {
						client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
						break;
					}
			}
			switch (cmd[i]) {
				case 'k':
					if (modeflag == 1) {
						channel->setMode(cmd[i], modeflag);
						channel->setKey(modeArgs[0]);
						client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +k " + modeArgs[0] + "\r\n");
						channel->broadcastMessageOps(":" + client.getNickname() + " has set " + channel->getName() + " key to " + modeArgs[0] + "\r\n", &client);
						modeArgs.erase(modeArgs.begin());
					}
					else {
						channel->setMode(cmd[i], modeflag);
						channel->setKey("");
						client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -k\r\n");
						channel->broadcastMessageOps(":" + client.getNickname() + " has removed " + channel->getName() + " key\r\n", &client);
					}
					break;
				case 'o':
					if (!channel->hasClient(server->findClientByNickname(modeArgs[0]))) {
							client.send("441 " + client.getNickname() + " " + channel->getName() + " " + modeArgs[0] + " :They aren't on that channel\r\n");
					} else if (modeflag == 1) {
							channel->addOperator(server->findClientByNickname(modeArgs[0]));
							client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +o " + modeArgs[0] + "\r\n");
							channel->broadcastMessageOps(":" + client.getNickname() + " MODE " + channel->getName() + " +o " + modeArgs[0] + "\r\n", &client);
					} else {
							channel->removeOperator(server->findClientByNickname(modeArgs[0]));
							client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -o " + modeArgs[0] + "\r\n");
							channel->broadcastMessageOps(":" + client.getNickname() + " has removed operator access to " + channel->getName() + " from " + modeArgs[0] + "\r\n", &client);
					}
					modeArgs.erase(modeArgs.begin());
					break;
				case 'l':
					if (modeflag == 1) {
						int limit = atoi(modeArgs[0].c_str());
						if (!limit) {
							client.send("502 " + client.getNickname() + " " + channel->getName() + " :Cannot change limit to that value\r\n");
						} else {
							channel->setMode(cmd[i], modeflag);
							channel->setUsersLimit(limit);
							client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +l " + modeArgs[0] + "\r\n");
							channel->broadcastMessageOps(":" + client.getNickname() + " has set user limit for " + channel->getName() + " to " + modeArgs[0] + "\r\n", &client);
						}
						modeArgs.erase(modeArgs.begin());
					} else {
						channel->setMode(cmd[i], modeflag);
						channel->setUsersLimit(-1);
						client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -l\r\n");
						channel->broadcastMessageOps(":" + client.getNickname() + " has removed user limit for " + channel->getName() + "\r\n", &client);
					}
					break;
				default:
					break;	
			}
		}
	}
}

void ModeCommandHandler::handle(Client& client, const Message& message) {
        // Basic MODE implementation - just acknowledge the mode request

        if (message.getParams().size() < 2 || !isChannel(message.getParams()[0])) {
                client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                return;
        }
		//there must be a channel passed from client to server, else it means the command isn't entered correctly.
        // For now, just send a generic response for user modes
        /*if (message.getParams()[0] == "*" || message.getParams()[0] == client.getNickname()) {
                client.send(":" + server.getServerName() + " 221 " + client.getNickname() + " +i\r\n");
        }*/
        Channel *channel = server.getChannel(message.getParams()[0]);
        if (!channel) {
                client.send("403 " + client.getNickname() + " " + message.getParams()[0] + " : Channel doesn't exist\r\n");
                return;
        }
        if (!channel->hasClient(&client)) {
                client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
                return;
        }
        if (!channel->isOperator(&client)) {
                client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
                return;
        }
        vector<string> modeArgs;
        if (message.getParams().size() > 2)
        {
                for (size_t i = 2; i < message.getParams().size(); i++) {
                        modeArgs.push_back(message.getParams()[i]);
                }
        }
        string cmd = message.getParams()[1];
        //parse through string, change flag whenever encounter +- else handle mode, take note of position of args in case mode needs args
        parseModeCmd(cmd, channel, client, &server, modeArgs);


        //set/remove invite only
        //set/remove restriction of topic setting to operators
        //set/remove channel key/password
        //give/take operator privilege
        //set/remove channel operator privilege


}

