/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helperFunction.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:59 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/16 18:12:36 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <iostream>

int	safe_socket(int domain, int type, int protocol) {
	int	fd = socket(domain, type, protocol);
	if (fd == -1) {
		std::cerr << "Failed to create socket: " << std::strerror(errno) << std::endl;
		return 1;
	}
	return fd;
}

/*
int bind(int socket, const struct sockaddr *address, socklen_t address_len)

The sin_port field is set to the port to which the application must bind. 
It must be specified in network byte order. If sin_port is set to 0, the 
caller leaves it to the system to assign an available port. 
The application can call getsockname() to discover the port number assigned.
*/
bool	safe_bind(int fd, sockaddr_in & addr) {
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		std::cerr << "Failed to bind: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

/*
listen only applies to stream sockets, it creates a connection request queue
size of backlog
*/
bool	safe_listen(int socket, int backlog) {
	if (listen(socket, backlog) == -1) {
		std::cerr << "Failed to listen: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

std::string	trim(std::string& s) {
	size_t	start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos) {
		s.clear();//all whitespace
		return "";
	}
	else {
		s = s.substr(start, end - start + 1);
		return s;
	}
}

std::string	cleanValue(std::string s) {
	//look for '//' preceded by ' ' or ';'
	size_t commentPos = std::string::npos;
	for (size_t i = 2; i < s.length(); ++i) {
		if (s[i] == '/' && s[i - 1] == '/' && (s[i - 2] == ' ' || s[i - 2] == ';')) {
			commentPos = i - 1;
			break;
		}
	}
	//use '#' if no comment '//' found
	size_t hashPos = s.find('#');
	if (hashPos != std::string::npos && (commentPos == std::string::npos || hashPos < commentPos))
		commentPos = hashPos;
	//remove comment
	if (commentPos != std::string::npos)
		s = s.substr(0, commentPos);
	//remove last semicolon
	size_t	semicolon = s.find_last_of(";");
	if (semicolon != std::string::npos)
		s = s.substr(0, semicolon);
	return trim(s);
}

void	shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients) {
	for (size_t i = 0; i < serverSockets.size(); i++)
		serverSockets[i]->closeSocket();
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete it->second;
	for (size_t i = 0; i < serverSockets.size(); ++i)
		delete serverSockets[i];
	std::cout << "🧼 Webserv shut down cleanly.\n";
}