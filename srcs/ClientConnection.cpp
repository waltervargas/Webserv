/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:34:47 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/25 15:27:25 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ClientConnection.hpp"
#include "../include/WebServ.hpp"
#include <poll.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <sys/socket.h> //internet protocol family
#include <string>
#include <vector>
#include <stdint.h>
#include <fstream>
#include <fcntl.h>

ClientConnection::ClientConnection(int fd) : _fd(fd), _state(READING_HEADERS), _contentLength(0),
	_bodyStart(0), _knownContentLength(false) {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK))
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
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

bool ClientConnection::readRequest(const ServerConfig& config) {
	char buffer[4096];
	int bytes = recv(_fd, buffer, sizeof(buffer), 0);;
	if (bytes <= 0) {
		std::cerr << "⚠️ Connection closed or recv failed during recvFullRequest\n";
		sendHtmlResponse(_fd, 400, getErrorPageBody(400, config));
		return false;
	}

	_buffer.insert(_buffer.end(), buffer, buffer + bytes);
	
	std::string reqStr(_buffer.begin(), _buffer.end());
	if (_state == READING_HEADERS) {
		size_t headerEnd = reqStr.find("\r\n\r\n");
		if (headerEnd != std::string::npos) {
			_state = READING_BODY;
			_bodyStart = headerEnd + 4; // Skip the \r\n\r\n
			// Found end of headers, check for Content-Length
			size_t contentLengthPos = reqStr.find("Content-Length:");
			if (contentLengthPos != std::string::npos) {
				size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
				size_t valueEnd = reqStr.find("\r\n", valueStart);
				std::string lengthStr = reqStr.substr(valueStart, valueEnd - valueStart);
				_contentLength = atoi(lengthStr.c_str());
				_knownContentLength = true;
			}
			else
				_contentLength = 0;
		}
	}
	
	//Step 2: read body if needed
	if (_state == READING_BODY) {
		size_t bodySize = (reqStr.size() - _bodyStart);
		if (!_knownContentLength || bodySize >= _contentLength) {
			_state = REQUEST_COMPLETE;
			return true;
		}
	}
	return false;
}
	
std::string ClientConnection::getRawRequest() const {
	return std::string(_buffer.begin(), _buffer.end());
}