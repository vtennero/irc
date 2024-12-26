#include <iostream>
#include "Server.hpp"
#include <cstdlib>

int main(int argc, char* argv[]) {
	std::cout << "[DEBUG] function main started" << std::endl;

	if (argc != 3) {
		std::cout << "[DEBUG] Invalid number of arguments" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	std::cout << "[DEBUG] Parsing arguments - Port: " << argv[1] << ", Password provided" << std::endl;
	int port = std::atoi(argv[1]);
	std::string password = argv[2];

	std::cout << "[DEBUG] Creating server instance" << std::endl;
	Server server(port, password);

	std::cout << "[DEBUG] Starting server" << std::endl;
	server.run();

	std::cout << "[DEBUG] Server shutdown, main ending" << std::endl;
	return 0;
}
