#include "ModeCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstdlib>

ModeCommandHandler::ModeCommandHandler(Server& server) : CommandHandler(server) {}

static bool isChannel(const string& channelName) {
		return !channelName.empty() && channelName[0] == '#';
}
//Skip all channels, then handle all modes, then match args
static void parseModeCmd(Message* message, Channel* channel, Client& client, Server* server, size_t argPos, vector<string>&modeArgs)
{
	int modeflag = 1;
	for (size_t i = 0; i < cmd.length(); ++i) {
		if (cmd[i] == '+') {
			modeflag = 1;
		}
		else if (cmd[i] == '-') {
			modeflag = 0;
		}
		//handle individual commands
		else if (cmd[i] == 'i' || cmd[i] == 't') {
			channel->setMode(cmd[i], modeflag);
			client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +" +  cmd[i] + "\r\n");
		}
		else if (cmd[i] == 'k' || cmd[i] == 'o' || cmd[i] == 'l') {
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

static size_t findMode(const Message& message) {
	//find mode operations immediately after channels, throw error if none
	for (size_t i = 1; i < message.getParams().size(); ++i) {
		if (message.getParams()[i][0] == '#') continue;
		if (message.getParams()[i][0] == '+' || message.getParams()[i][0] == '-') {
			return i;
		} else {
			return 0;
		}
	}
	return 0;
}

void ModeCommandHandler::handle(Client& client, const Message& message) {
        // Caters to irssi mode command at connect, we don't process if it's mode user
		if (findClientByNickname(message.getParams()[0]) != NULL) {return;}
        if (message.getParams().size() < 2 || !isChannel(message.getParams()[0])) {
                client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                return;
        }
		//Things to check for: 
		//2. find mode operations, throw error if none
		//3. read until space, continue
		//4. once space followed by non +/-, the remaining are arguments
		//5. ignore if not valid options
		//6. use args sequentially based on modes that need them, else throw error but continue
		//Checks if server exists
        Channel *channel = server.getChannel(message.getParams()[0]);
        if (!channel) {
                client.send("403 " + client.getNickname() + " " + message.getParams()[0] + " : Channel doesn't exist\r\n");
                return;
        }
		//check if client in channel, auth, operator
        if (!channel->hasClient(&client)) {
                client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
                return;
        }
		if (!client.isAuthenticated()) {
			client.send("481 " + client.getNickname() + " " + message.getParams()[0] + " : You're not authenticated\r\n");
			return;
		}
        if (!channel->isOperator(&client)) {
                client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
                return;
        }
		size_t argPos = findMode(message);
		if (!argPos) {
			//return error if no mode operations found
			client.send("461 " + client.getNickname() + " " + message.getParams()[0] + " : No mode operations found\r\n");
			return;
		}
		//skip past mode operations
		while (argPos < message.getParams().size() && message.getParams()[argPos][0] == '+' || message.getParams()[argPos][0] == '-') {
			argPos++;
		}
		//store arguments, any options encountered from here on will be treated as arg
        vector<string> modeArgs;
        if (message.getParams().size() > 2)
        {
                for (size_t i = argPos; i < message.getParams().size(); ++i) {
					if (message.getParams()[i][0] == '#' || message.getParams()[i][0] == '+' || message.getParams()[i][0] == '-') {
						continue ;
					}
					//s
					cout << "args are: " << message.getParams()[i] << endl;
                    modeArgs.push_back(message.getParams()[i]);
                }
        }

        
        parseModeCmd(message, channel, client, &server, argPos, modeArgs);


        //set/remove invite only
        //set/remove restriction of topic setting to operators
        //set/remove channel key/password
        //give/take operator privilege
        //set/remove channel operator privilege


}

