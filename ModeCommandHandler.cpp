#include "ModeCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstdlib>

ModeCommandHandler::ModeCommandHandler(Server& server) : CommandHandler(server) {}
//CHECK FOR AUTHENTICATION
static bool isChannel(const string& channelName) {
		return !channelName.empty() && channelName[0] == '#';
}

static void handleSimpleMode(char mode, bool modeflag, Channel* channel, Client& client) {
    channel->setMode(mode, modeflag);
    client.send(":" + client.getNickname() + " MODE " + channel->getName() + " " + (modeflag ? "+" : "-") + mode + "\r\n");
}

static void handleKeyMode(bool modeflag, const string& key, Channel* channel, Client& client) {
    if (modeflag == 1) {
        channel->setKey(key);
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +k " + key + "\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " has set " + channel->getName() + " key to " + key + "\r\n", &client);
    } else {
        channel->setKey("");
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -k\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " has removed " + channel->getName() + " key\r\n", &client);
    }
}

static void handleOperatorMode(bool modeflag, const string& targetNick, Channel* channel, Client& client, Server* server) {
    Client* targetClient = server->findClientByNickname(targetNick);
    if (!targetClient) {
        client.send("441 " + client.getNickname() + " " + channel->getName() + " " + targetNick + " :They aren't on that channel\r\n");
        return;
    }

    if (modeflag == 1) {
        channel->addOperator(targetClient);
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +o " + targetNick + "\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " MODE " + channel->getName() + " +o " + targetNick + "\r\n", &client);
    } else {
        channel->removeOperator(targetClient);
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -o " + targetNick + "\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " has removed operator access to " + channel->getName() + " from " + targetNick + "\r\n", &client);
    }
}

static void handleLimitMode(bool modeflag, const string& limitStr, Channel* channel, Client& client) {
    int limit = atoi(limitStr.c_str());
    if (modeflag == 1 && limit <= 0) {
        client.send("502 " + client.getNickname() + " " + channel->getName() + " :Cannot change limit to that value\r\n");
    } else if (modeflag == 1) {
        channel->setUsersLimit(limit);
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " +l " + limitStr + "\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " has set user limit for " + channel->getName() + " to " + limitStr + "\r\n", &client);
    } else {
        channel->setUsersLimit(-1);
        client.send(":" + client.getNickname() + " MODE " + channel->getName() + " -l\r\n");
        channel->broadcastMessageOps(":" + client.getNickname() + " has removed user limit for " + channel->getName() + "\r\n", &client);
    }
}

static void parseModeCmd(const string& cmd, Channel* channel, Client& client, Server* server, vector<string>& modeArgs) {
    bool modeflag = true; // true for '+', false for '-'
    for (size_t i = 0; i < cmd.length(); ++i) {
        char mode = cmd[i];

        if (mode == '+') {
            modeflag = true;
        } else if (mode == '-') {
            modeflag = false;
        } else {
            switch (mode) {
                case 'i':
                case 't':
                    handleSimpleMode(mode, modeflag, channel, client);
                    break;
                case 'k':
                    if (modeArgs.empty()) {
                        client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                    handleKeyMode(modeflag, modeArgs[0], channel, client);
                    modeArgs.erase(modeArgs.begin());
                    break;
                case 'o':
                    if (modeArgs.empty()) {
                        client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                    handleOperatorMode(modeflag, modeArgs[0], channel, client, server);
                    modeArgs.erase(modeArgs.begin());
                    break;
                case 'l':
                    if (modeArgs.empty()) {
                        client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                    handleLimitMode(modeflag, modeArgs[0], channel, client);
                    modeArgs.erase(modeArgs.begin());
                    break;
                default:
                    // Ignore invalid modes or throw an error
                    break;
            }
        }
    }
}
//Skip all channels, then handle all modes, then match args
/*static void parseModeCmd(const string& cmd, Channel* channel, Client& client, Server* server, vector<string>&modeArgs)
{
	int modeflag = 1;
	//go through options
//	for (size_t j = 0; j < modeOps.size(); ++j) {
//		string cmd = modeOps[j];
		cout << cmd << endl;
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
//		}
	}
}*/

/*static size_t findMode(const Message& message) {
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
}*/

void ModeCommandHandler::handle(Client& client, const Message& message) {
        // skips initial mode commands by irssi
		if (message.getParams()[0] == "*" || server.findClientByNickname(message.getParams()[0]) != NULL) {
			return;
		}
        if (message.getParams().size() < 2 || !isChannel(message.getParams()[0])) {
                client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                return;
        }
		//Checks if channel exists
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
		//store arguments, any options encountered from here on will be treated as arg
        vector<string> modeArgs;
        for (size_t i = 2; i < message.getParams().size(); i++) {
			modeArgs.push_back(message.getParams()[i]);
        }
        parseModeCmd(message.getParams()[1], channel, client, &server, modeArgs);
}