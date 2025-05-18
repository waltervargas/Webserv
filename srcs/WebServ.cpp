/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:29 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/18 18:30:52 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

int g_signal = -1;

void handleSignal(int signal) {
	if (signal == SIGINT) {
		std::cout << "\n🛑 Ctrl+C detected! Shutting down server...\n";

		if (g_signal != -1) {
			std::cout << "🔒 Server socket closed\n";
		}
		g_signal = 0;
	}
}

void	handleNewClient(ServerSocket* server, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, std::map<int, ServerSocket*>& clientToServer) {
	int	client_fd = server->acceptClient();
	if (client_fd == -1) {
		std::cerr << "❌ Failed to accept client\n";
		return;
	}
	ClientConnection* client = new ClientConnection(client_fd);
	pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	client_pfd.revents = 0;
	fds.push_back(client_pfd);
	clients[client_fd] = client;
	clientToServer[client_fd] = server;
	std::cout << "A new client has been connected: " << client_fd << std::endl;
}

void	handleExistingClient(int fd, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, size_t& i, const ServerConfig& config) {
	std::map<int, ClientConnection*>::iterator it = clients.find(fd);
	if (it == clients.end()) {
		std::cerr << "❌ Received an event for an unknow fd: " << fd << std::endl;
		return;
	}
	ClientConnection* client = it->second;
	std::string request = client->recvFullRequest(fd);
	if (request.empty()) {
		std::cerr << "❌ Empty or invalid HTTP request\n";
		close(fd);
		delete client;
		clients.erase(it);
		fds.erase(fds.begin() + i);
		--i;
		return;
	}
	Request	req(request);
	std::string method = req.getMethod();
	std::string path = req.getPath();
	
	std::cout << "📨 " << method << " " << path << std::endl;
	if (method == "POST" && path == "/upload") {
		std::cout << "handing upload\n" << std::endl;
		handleUpload(request, fd, config);
	}
	else
		serveStaticFile(path, fd, config);

	close(fd);
	delete client;
	clients.erase(it);
	fds.erase(fds.begin() + i);
	--i;
}

int	init_webserv(std::string configPath) {
	ConfigParser	parser;
	std::map<int, ServerSocket*> clientToServer;
	try {
		parser.parseFile(configPath);
	}
	catch (const std::exception& e) {
		std::cerr << "❌ Error while parsing config: " << e.what() << std::endl;
		return 1;
	}
//	parser.print();

	const	std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Error: No servers found in config file.\n";
		return 1;
	}
	std::vector<struct pollfd> fds;
	std::vector<ServerSocket*> serverSockets;
	std::map<int, ServerSocket*> fdToSocket;
	std::map<int, ClientConnection*> clients;
	
	for (size_t i = 0; i < servers.size(); ++i) {
		const std::string& host = servers[i].host;
		int port = servers[i].port;

		ServerSocket*	server = new ServerSocket();
		if (!server->init(port, host)) {
			delete server;
			std::cerr << "❌ Failed to initialise server on port: " << port << std::endl;
			continue;
		}
		server->setConfig(servers[i]);
		int	fd = server->getFD();
	
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		fds.push_back(pfd);
		
//		std::cout << "\n======================" << std::endl;
//		server->getConfig().print();
		serverSockets.push_back(server);
		fdToSocket[fd] = server;
		std::cout << "✅ Server is up at http://" << host << ":" << port << std::endl;
	}

	while (g_signal != 0) {
		for (size_t i = 0; i < fds.size(); ++i) {
			int fd = fds[i].fd;
			bool known = fdToSocket.count(fd) || clients.count(fd);
			if (fd <0 || !known) {
				std::cerr << "🧽 Removing unknown or invalid fd: " << fd << std::endl;
				close(fd);
				fds.erase(fds.begin() + i);
				--i;
			}
		}
		//reset revents before poll
		for (size_t i = 0; i < fds.size(); ++i)
			fds[i].revents = 0;
		//safe to call poll()
		int openAndReadyFDs = poll(&fds[0], fds.size(), -1);
		if (openAndReadyFDs < 0) {
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				if (fdToSocket.count(fds[i].fd))
					handleNewClient(fdToSocket[fds[i].fd], fds, clients, clientToServer);
				else if (clients.count(fds[i].fd))
					handleExistingClient(fds[i].fd, fds, clients, i, clientToServer[fds[i].fd]->getConfig());
				else {
					std::cerr << "⚠️ POLLIN on unknown fd " << fds[i].fd << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				}
			}
		}
	}
	shutDownWebserv(serverSockets, clients);
	std::cout << "👋 Bye bye!\n";
	return 0;
}

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
