#include "include/ServerSocket.hpp"
#include "include/ClientConnection.hpp"
#include <iostream>


int main() {
	ServerSocket	server;
	if (!server.init(8081)) {
		std::cerr << "Failed to initialise server" << std::endl;
		return 1;
	}
	std::cout << "Server is up and running" << std::endl;

	//handles only 1 client
	int client_fd = server.acceptClient();
	if (client_fd == -1) {
		std::cerr << "Failed to accept client" << std::endl;
		return 1;
	}
	
	ClientConnection	client(client_fd);
	if (!client.receiveMessage()) {
		std::cerr << "Failed to receive message from client" << std::endl;
		return 1;
	}
	std::cout << "Message from client:\n" << client.getMessage() << std::endl;
	return 0;
}