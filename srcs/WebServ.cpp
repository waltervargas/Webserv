/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:29 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/27 15:10:22 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

int g_signal = -1;

// Signal handler for SIGINT and SIGTERM; sets global flag to initiate server shutdown.
void handleSignal(int signal) {
	if (signal == SIGINT) {
		std::cout << "\n🛑 Ctrl+C detected! Shutting down server...\n";

		if (g_signal != -1) {
			std::cout << "🔒 Server socket closed\n";
		}
		g_signal = 0;
	}
}

/*
Initializes the server using the config file, sets up server sockets and poll monitoring,
and runs the main poll loop to handle client connections and requests.
*/
int	init_webserv(std::string configPath) {
	ConfigParser	parser;

	try {
		parser.parseFile(configPath);
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
	//check for Duplicate Host Port Pairs
	checkDuplicateHostPortPairs(servers);
	
	std::vector<ServerSocket*> serverSockets;
	std::vector<struct pollfd> fds;
	std::map<int, ServerSocket*> fdToSocket;
	if (!initialiseSockets(servers, serverSockets, fds, fdToSocket))
		return 1;

	std::map<int, ClientConnection*> clients;
	std::map<int, ServerSocket*> clientToServer;
	
	runEventLoop(fds, fdToSocket, clients, clientToServer);

	shutDownWebserv(serverSockets, clients);
	std::cout << "👋 Bye bye!\n";
	return 0;
}

/*
Main entry point: sets up signal handlers, parses CLI arguments for the config file path,
and starts the server using init_webserv().
*/
int main(int ac, char **av) {

	signal(SIGINT, handleSignal); //handle Contrl + C
	signal(SIGTERM, handleSignal); //handle kill <pid>
	std::string configPath;
	std::cout << "		My Webserv in C++98" << std::endl;
	std::cout << "--------------------------------------------------\n " << std::endl;

	if (ac == 1)
		configPath = "conf/default.conf";
	else if (ac == 2)
		configPath = av[1];
	else{
		std::cout << "Too many arguments!!\nUsage: ./webserv [config_file]" << std::endl;
		return (EXIT_FAILURE);
	}
	std::cout << "Using configuration file: " << configPath << std::endl;

	return (init_webserv(configPath));
}
