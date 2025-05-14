/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:34:47 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/14 16:18:22 by kbolon           ###   ########.fr       */
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

bool recvExact(int fd, void* buf, size_t len) {
	size_t	total = 0;
	char* ptr = static_cast<char*>(buf);
	while (total < len) {
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLIN;
		int ready = poll(&pfd, 1, 1000); //timeout after 1000ms
		if (ready <= 0) {
			return false;
		}
		ssize_t	received = recv(fd, ptr + total, len - total, 0);
		if (received <= 0) {
			return false;
		}
		total += received;
	}
	return true;
}

ClientConnection::ClientConnection(int fd) : _fd(fd) {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK))
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

/*
-We use non-blockeing recv() safely
-server waits with poll() when necessary
-server reads one full file per client connection
-gracefully handles slow clients and disconnects
-exits cleanly after each transfer
*/
bool	ClientConnection::receiveMessage() {
	while (true) {
		const size_t	bufferSize = 1024;
		char buffer[bufferSize];
		mkdir("test/output", 0777); //ignores if already exists


		struct pollfd pfd;
		pfd.fd = _fd;
		pfd.events = POLLIN;

		int ready = poll(&pfd, 1, 1000); //timeout after 1000ms
		if (ready <= 0) {
			std::cerr << "⚠️ No data from client or poll failed in ClientConnection\n";
			return true; //keep connection open
		}
		if (!(pfd.revents & POLLIN)) 
			return true;
	
		//Step 1: read filename length
		uint8_t	nameLen = 0;
		ssize_t bytes = recv(_fd, &nameLen, sizeof(nameLen), 0);
		if (bytes == 0) {
			std::cerr << "✅ Client closed the connection\n";
			return false;
		}
		else if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return true; //no data available yet
			std::cerr << "❌ Client connection error " << strerror(errno) << "\n";
			return false;
		}
		if (nameLen > 255) {
			std::cerr << "File name is too long\n";
			return false;
		}
		//Step 2: Read filename
		char	nameBuffer[256] = {0};
		if (!recvExact(_fd, nameBuffer, nameLen)) {
			std::cerr << "❌ Failed to receive full filename\n";
			return false;
		}
		std::string	fileName(nameBuffer, nameLen);
		std::cout << "🧾 Receiving file: " << fileName << std::endl;
	
		//Step 3: Read file size
		uint32_t	fileSize = 0;
		if (!recvExact(_fd, &fileSize, sizeof(fileSize))) {
			std::cerr << "❌ Failed to receive file size\n";
			return false;
		}

		//Step 4: Receive file content
		std::string fullPath =  "test/output/" + fileName;
		std::ofstream	output(fullPath.c_str(), std::ios::binary);
		if (!output) {
			std::cerr << "Failed to open file for writing\n";
			return false;
		}

		//read exactly fileSize bytes
		uint32_t remaining = fileSize;
		while (remaining > 0) {
			ssize_t toRead = (remaining > bufferSize) ? bufferSize : remaining;
			int wait = poll(&pfd, 1, 1000);
			if (wait <= 0) {
				std::cerr << "❌ Timeout or poll error during file receive\n";
				return false;
			}
			if (!(pfd.revents & POLLIN))
				continue;
			ssize_t	received = recv(_fd, buffer, toRead, 0);
			if (received <= 0) {
				std::cerr << "Receive failed or connection was interrupted\n";
				return false;
			}
			output.write(buffer, received);
			if (!output){
				std::cerr << "Failed to write to file\n";
				return false;
			}
			remaining -= received;
		}
		output.close();
		std::cout << "✅ File saved: " << fileName << std::endl;
		continue;
	}
	return true;
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
