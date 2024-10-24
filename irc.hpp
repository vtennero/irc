#include <vector>
#include <iostream>
#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// Forward declarations


// Main server class
class Server;

// Represents a connected client
class Client;
    // Will manage individual client connections and state

// Represents an IRC channel
class Channel;
    // Will manage channel state, users, and operations

// Represents a parsed IRC message
class Message;
    // Will handle parsing and representing IRC protocol messages

// Base class for command handlers
class CommandHandler;
    // Base class for handling specific IRC commands

// Specific command handlers (inherit from CommandHandler)
// class NickCommandHandler : public CommandHandler {};
// class UserCommandHandler : public CommandHandler {};
// class JoinCommandHandler : public CommandHandler {};
// class PartCommandHandler : public CommandHandler {};
// class PrivmsgCommandHandler : public CommandHandler {};
// class QuitCommandHandler : public CommandHandler {};
// class KickCommandHandler : public CommandHandler {};
// class InviteCommandHandler : public CommandHandler {};
// class TopicCommandHandler : public CommandHandler {};
// class ModeCommandHandler : public CommandHandler {};

// Utility classes
class Logger {
    // Will handle logging for debugging and monitoring
};

class ConfigManager {
    // Will manage server configuration
};

// Manager classes to handle collections and operations
class ClientManager {
    // Will manage all connected clients
};

class ChannelManager {
    // Will manage all active channels
};

class CommandManager {
    // Will dispatch commands to appropriate handlers
};

// Network utility class
class NetworkUtils {
    // Will contain utility functions for network operations
};