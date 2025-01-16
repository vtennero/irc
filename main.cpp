#include <iostream>
#include "Server.hpp"
#include <cstdlib>

int main(int argc, char* argv[]) {
	cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " started" << endl;

	if (argc != 3) {
		cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " Invalid number of arguments" << endl;
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Usage: " << argv[0] << " <port> <password>" << endl;
		return 1;
	}

	cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " Parsing arguments - Port: " << argv[1] << ", Password provided" << endl;
	int port = atoi(argv[1]);
	string password = argv[2];

	cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " Creating server instance" << endl;
	Server server(port, password);

	if (server.getSocket() == -1) {  // Add this getter to Server class
		cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " Server failed to initialize" << endl;
		return 1;
	}

	cout << MAGENTA "[" << __PRETTY_FUNCTION__ << "]" RESET " Starting server" << endl;
	server.run();

	return 0;
}
