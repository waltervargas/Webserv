/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/10 18:20:33 by kbolon           ###   ########.fr       */
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

/*
this is the main I/O loop with poll()
*/
void	runEventLoop(	std::vector<struct pollfd>& fds, 
						std::map<int, ServerSocket*>& fdToSocket,
						std::map<int, ClientConnection*>& clients, 
						std::map<int, ServerSocket*>& clientToServer) {

	while (g_signal != 0) {
		//safe to call poll()
		// revents will be automatically set by poll(), no need to reset manually
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0) {
			if (errno == EINTR)
				continue;
			std::cerr << "❌ Poll() error: " << strerror(errno) << std::endl;
			break;
		}
		//handle ready FD's (if there is data to read)
		for (size_t i = 0; i < fds.size(); ++i) {
			//revents field is declared as a short
			short tempRevent = fds[i].revents;
			int fd = fds[i].fd;
			
			if (tempRevent & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "❌ Error or hangup on client side\n" << fd << std::endl;
				close(fd);
				fds.erase(fds.begin() + i);
				--i;
				continue;
			}
			if (tempRevent & POLLIN) {
				if (fdToSocket.count(fd))
					handleNewClient(fdToSocket[fd], fds, clients, clientToServer);
				else if (clients.count(fd))
					handleExistingClient(fd, fds, clients, i, clientToServer[fd]->getConfig());
				else {
					std::cerr << "⚠️ POLLIN on unknown fd " << fd << std::endl;
					close(fd);
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
This function finds the corresponding file descriptor in the ClientConnection map,
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
	//Step 1: read data from client
	client->recvFullRequest(fd, config);
	//Step 2: Wait for all data to be received
	if (!client->isRequestComplete())
		return;
	
	std::string request = client->getRawRequest();
	//parset the HTTP item into a Request object & extract methods and path

	Request	req(request);
	std::string method = req.getMethod();
	std::string path = req.getPath();
	std::string interpreter;
	
	std::cout << "📨 " << method << " " << path << std::endl;
	LocationConfig	location = matchLocation(path, config);
	
	//return error page if specified in location block
	if (location.returnStatusCode != 0) {
		std::string body = getErrorPageBody(location.returnStatusCode, config);
		sendHtmlResponse(fd, location.returnStatusCode, body);
		goto cleanup;
	}
	//handle uploads
	if (method == "POST" && path == "/upload") {
		std::cout << "handling upload\n" << std::endl;
		//handle file uploads
		handleUpload(request, fd, config);
		goto cleanup;
	}
	//check for CGI interpreter (.py, .php, etc.)
	interpreter = getInterpreter(path, config);
	if (!interpreter.empty()) {
		//if CGI, run it
		handleCgi(req, fd, config, interpreter);
		goto cleanup;
	}
	//default: serve static
	serveStaticFile(path, fd, config);
	std::cout << "🧪 getPath: " << req.getPath() << "\n";
	std::cout << "🧪 getQuery: " << req.getQuery() << "\n";
	cleanup:
		close(fd);
		delete client;
		clients.erase(it);
		fds.erase(fds.begin() + i);
		--i;
}