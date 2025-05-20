/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:29 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/20 10:33:28 by kbolon           ###   ########.fr       */
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
Accepts a new client connection, creates a new ClientConnection object to manage communication,
sets up a pollfd struct for the client, and adds it to the poll list.
*/
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

/*
When poll() identifies data to be read on a client socket, this function is called.
It finds the corresponding file descriptor in the ClientConnection map,
parses and handles the HTTP request using the appropriate handler (static, CGI, or upload),
and then cleans up the client connection.
*/
void	handleExistingClient(int fd, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, size_t& i, const ServerConfig& config) {
	//finds the FD in the ClientConnection container
	std::map<int, ClientConnection*>::iterator it = clients.find(fd);
	// Defensive check: poll gave us a fd we don't know? Exit function.
	if (it == clients.end()) {
		std::cerr << "❌ Received an event for an unknown fd: " << fd << std::endl;
		return;
	}
	ClientConnection* client = it->second;
	//read the HHTP request from the socket
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
	//parset the HTTP item into a Request object & extract methods and path
	Request	req(request);
	std::string method = req.getMethod();
	std::string path = req.getPath();
	
	std::cout << "📨 " << method << " " << path << std::endl;
	if (method == "POST" && path == "/upload") {
		std::cout << "handling upload\n" << std::endl;
		//handle file uploads
		handleUpload(request, fd, config);
	}
	else {
		//check for CGI interpreter (.py, .php, etc.)
		std::string interpreter = getInterpreter(path, config);
		if (!interpreter.empty())
			//if CGI, run it
			handleCgi(req, fd, config, interpreter);
		else
			//if not CGI, use default file
			serveStaticFile(path, fd, config);
	}
	close(fd);
	delete client;
	clients.erase(it);
	fds.erase(fds.begin() + i);
	--i;
}

/*
Initializes the server using the config file, sets up server sockets and poll monitoring,
and runs the main poll loop to handle client connections and requests.

poll() is a system call that allows us to monitor multiple FDs to see if they are ready for I/O,
without blocking on just one FD.
*/
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
	// list of FDs to monitor with poll()
	std::vector<struct pollfd> fds;
	std::vector<ServerSocket*> serverSockets;
	std::map<int, ServerSocket*> fdToSocket;
	std::map<int, ClientConnection*> clients;
	
	// Creates a ServerSocket, binds/listens on specified host/port, then configures the server.
	// Adds the server FD to the poll list to monitor for POLLIN events,
	// and tracks the ServerSocket for later access and cleanup.
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
		//reset revents before poll as revents tells poll() why the FD is ready
		for (size_t i = 0; i < fds.size(); ++i)
			fds[i].revents = 0;
		//safe to call poll()
		int openAndReadyFDs = poll(&fds[0], fds.size(), -1);
		if (openAndReadyFDs < 0) {
			break;
		}
		//handle ready FD's (if there is data to read)
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
