#include <iostream>
#include "Server.hpp"
#include <cstdlib>

int main(int argc, char* argv[]) {
	cout << "[DEBUG] function main started" << endl;

	if (argc != 3) {
		cout << "[DEBUG] Invalid number of arguments" << endl;
		cerr << "Usage: " << argv[0] << " <port> <password>" << endl;
		return 1;
	}

	cout << "[DEBUG] Parsing arguments - Port: " << argv[1] << ", Password provided" << endl;
	int port = atoi(argv[1]);
	string password = argv[2];

	cout << "[DEBUG] Creating server instance" << endl;
	Server server(port, password);

	cout << "[DEBUG] Starting server" << endl;
	server.run();

	cout << "[DEBUG] Server shutdown, main ending" << endl;
	return 0;
}
