/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:34:47 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/18 18:30:23 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ClientConnection.hpp"
#include <poll.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <sys/socket.h> //internet protocol family
#include <string>
#include <stdint.h>
#include <fstream>
#include <fcntl.h>

ClientConnection::ClientConnection(int fd) : _fd(fd) {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK))
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

std::string	ClientConnection::getMessage() const {
	return _message;
}

int	ClientConnection::getFd() const {
	return _fd;
}

void	ClientConnection::closeConnection() {
	if (_fd != -1)
		close(_fd);
}

ClientConnection::~ClientConnection() {
	closeConnection();
}

/*
-We use non-blockeing recv() safely
-server waits with poll() when necessary
-server reads one full file at a time per client connection
-gracefully handles slow clients and disconnects
-exits cleanly after each transfer
*/
std::string ClientConnection::recvFullRequest(int client_fd) {
	std::string request;
	char buffer[4096];
	int bytes = 0;
	size_t totalContentLength = 0;
	bool headersParsed = false;
	size_t headerEnd = std::string::npos;
	int	totalWait = 0;
	const int pollStep = 300; //ms
	const int maxWait = 3000; //3 seconds

	// First, receive the headers
	while (totalWait < maxWait) {
		struct pollfd pfd;
		pfd.fd = client_fd;
		pfd.events = POLLIN;

		int ready = poll(&pfd, 1, pollStep);
		if (ready > 0 && (pfd.revents & POLLIN)) {
			bytes = recv(client_fd, buffer, sizeof(buffer), 0);
			if (bytes <= 0) {
				std::cerr << "⚠️ Connection closed or recv failed during recvFullRequest\n";
				break;
			}
			request.append(buffer, bytes);
		}
		totalWait += pollStep;
		if (!headersParsed) {
			headerEnd = request.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				headersParsed = true;
				// Found end of headers, check for Content-Length
				size_t contentLengthPos = request.find("Content-Length:");
				if (contentLengthPos != std::string::npos) {
					size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
					size_t valueEnd = request.find("\r\n", valueStart);
					std::string lengthStr = request.substr(valueStart, valueEnd - valueStart);
					totalContentLength = atoi(lengthStr.c_str());
					if (totalContentLength > 0)
						std::cout << "Content-Length in request: " << totalContentLength << " bytes" << std::endl;
				}
			}
		}
		// Parse headers if not already parsed
		// Once headers are parsed, wait until full body is read
		if (headersParsed && totalContentLength > 0) {
			// Calculate how much of the body we've already received
			size_t bodyStart = headerEnd + 4; // Skip the \r\n\r\n
			size_t bodyReceived = (request.size() - bodyStart);
			if (bodyReceived >= totalContentLength) {
				break; //full request body received
			}
		}
		if (headersParsed && totalContentLength == 0)
			break;
	}

	// Final validation
	if (!headersParsed) {
		std::cerr << "❌ Failed to parse headers\n";
		return "";
	}
	if (totalContentLength > 0) {
		size_t bodyStart = headerEnd + 4;
		size_t bodyReceived = request.size() - bodyStart;
		if (bodyReceived < totalContentLength) {
			std::cerr << "❌ Incomplete body (received " << request.size() - bodyStart
				<< " of " << totalContentLength << " bytes)\n";
			return "";
		}
	}
	return request;
}
