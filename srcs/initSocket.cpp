/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/26 15:51:44 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

/*
This function takes the groups of Servers, Sockets etc
and tries to initilise them
*/

bool initialiseSockets(const std::vector<ServerConfig>& servers, std::vector<ServerSocket*>& serverSockets,
			std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket) {
	// Creates a ServerSocket, binds/listens on specified host/port, then configures the server.
	// Adds the server FD to the poll list to monitor for POLLIN events,
	// and tracks the ServerSocket for later access and cleanup.
	for (size_t i = 0; i < servers.size(); ++i) {
		for (size_t j = 0; j < servers[i].ports.size(); ++j) {
			ServerSocket*	server = new ServerSocket();
			if (!server->init(servers[i].ports[j], servers[i].host)) {
				delete server;
				std::cerr << "❌ Failed to initialise server on port: " << servers[i].ports[j] << std::endl;
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
			std::cout << "✅ Server is up at http://" << servers[i].host << ":" << servers[i].ports[j] << std::endl;
		}
	}
	if (serverSockets.empty())
		return false;
	return true;
}

void	runEventLoop(std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket,
					std::map<int, ClientConnection*>& clients, std::map<int, ServerSocket*>& clientToServer) {

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
	if (!client->readRequest(config)) //not done yet, wait for next POLLIN
		return;
	
	std::string rawRequest = client->getRawRequest();
	//parset the HTTP item into a Request object & extract methods and path
	Request	req(rawRequest);
	std::string method = req.getMethod();
	std::string path = req.getPath();
	
	std::cout << "📨 " << method << " " << path << std::endl;
	LocationConfig	location = matchLocation(path, config);
	
	if (location.returnStatusCode != 0) {
		std::string body = getErrorPageBody(location.returnStatusCode, config);
		sendHtmlResponse(fd, location.returnStatusCode, body);
		close(fd);
		delete client;
		clients.erase(it);
		fds.erase(fds.begin() + i);
		--i;
		return;
	}
	if (method == "POST" && path == "/upload") {
		std::cout << "handling upload\n" << std::endl;
		//handle file uploads
		handleUpload(rawRequest, fd, config);
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