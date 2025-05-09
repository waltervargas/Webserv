#include "include/ServerSocket.hpp"
#include "include/ClientConnection.hpp"
#include "include/ConfigParser.hpp"
#include <iostream>
#include <fstream>


int main(int ac, char **av) {
	if (ac != 2) {
		std::cerr << "❌ Usage: ./program configFile\n";
		return 1;
	}
	ConfigParser	parser;
	try {
		parser.parseFile(av[1]);
	}
	catch (const std::exception& e) {
		std::cerr << "❌ Error while parsing config: " << e.what() << std::endl;
		return 1;
	}
	parser.print();
	const	std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Error: No servers found in config file.\n";
		return 1;
	}
	//initialise the first server
	int port = servers[0].port;
	std::cout << "Starting server on port: " << port << std::endl;

	ServerSocket	server;
	if (!server.init(port)) {
		std::cerr << "❌ Failed to initialise server\n";
		return 1;
	}
	std::cout << "✅ Server is up and running\n";

	//handles only 1 client (blocking for now)
	int client_fd = server.acceptClient();
	if (client_fd == -1) {
		std::cerr << "❌ Failed to accept client\n";
		return 1;
	}
	
	ClientConnection	client(client_fd);
	if (!client.receiveMessage()) {
		std::cerr << "❌ Failed to receive message from client\n";
		return 1;
	}
	std::cout << "📩 Message from client:\n" << client.getMessage() << std::endl;
	return 0;
}