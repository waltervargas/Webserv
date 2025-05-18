/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helperFunction.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:59 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/18 17:19:34 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"
#include "../include/ServerConfig.hpp"
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

std::string getContentType(const std::string& path) {
	if (path.find(".html") != std::string::npos) return "text/html; charset=UTF-8";
	if (path.find(".css") != std::string::npos) return "text/css; charset=UTF-8";
	if (path.find(".js") != std::string::npos) return "application/javascript; charset=UTF-8";
	if (path.find(".png") != std::string::npos) return "image/png";
	if (path.find(".jpg") != std::string::npos) return "image/jpeg";
	return "application/octet-stream";
}

void serveStaticFile(std::string path, int client_fd, const ServerConfig &config) {
	if (path == "/")
		path = "/" + config.index;

	std::string fullPath = config.root + path;
	std::ifstream file(fullPath.c_str());
	if (!file.is_open()) {
		std::string errorPath = config.root + "/error/404.html";
		std::ifstream errorFile(errorPath.c_str());

		if (errorFile.is_open()) {
			std::string errorBody((std::istreambuf_iterator<char>(errorFile)), std::istreambuf_iterator<char>());
			errorFile.close();

			std::ostringstream oss;
			oss << "HTTP/1.1 404 Not Found\r\n";
			oss << "Content-Type: text/html; charset=UTF-8\r\n";
			oss << "Content-Length: " << errorBody.size() << "\r\n";
			oss << "Connection: close\r\n\r\n";
			oss << errorBody;
			std::string response = oss.str();
			ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
			if (sent != (ssize_t)response.size()) {
				std::cerr << "❌ Failed to send complete error response: " << sent << " of " << response.size() << " bytes" << std::endl;
			}
		} else {
			std::string fallback =
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/plain; charset=UTF-8\r\n"
				"Content-Length: 21\r\n"
				"Connection: close\r\n\r\n"
				"404 - File not found!";
			ssize_t sent = send(client_fd, fallback.c_str(), fallback.size(), 0);
			if (sent != (ssize_t)fallback.size()) {
				std::cerr << "❌ Failed to send complete fallback error response: " << sent << " of " << fallback.size() << " bytes" << std::endl;
			}
		}
		return;
	}

	std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	std::ostringstream oss;
	oss << "HTTP/1.1 200 OK\r\n";
	oss << "Content-Type: " << getContentType(path) << "\r\n";
	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "Connection: close\r\n\r\n";
	oss << body;
	std::string response = oss.str();
	ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
	if (sent != (ssize_t)response.size()) {
		std::cerr << "❌ Failed to send complete static file response: " << sent << " of " << response.size() << " bytes" << std::endl;
	}
}

/*
* This is a complete rewrite of the upload handler using a direct byte-by-byte approach
* designed specifically to handle binary file uploads correctly.
*/
void handleUpload(const std::string &request, int client_fd, const ServerConfig &config) {

	// Find the filename first
	size_t filenamePos = request.find("filename=\"");
	if (filenamePos == std::string::npos) {
		std::cerr << "❌ No filename found in request.\n";
		return;
	}

	size_t filenameStart = filenamePos + 10; // Length of "filename=\""
	size_t filenameEnd = request.find("\"", filenameStart);
	if (filenameEnd == std::string::npos) {
		std::cerr << "❌ Invalid filename format.\n";
		return;
	}

	std::string filename = request.substr(filenameStart, filenameEnd - filenameStart);
	if (filename.empty()) {
		std::cerr << "❌ Empty filename.\n";
		return;
	}
	std::cout << "Filename: " << filename << std::endl;

	// Find the double CRLF that marks the start of the file data
	// This is the most reliable marker - find the Content-Type header first,
	// then find the blank line that follows it
	size_t contentTypePos = request.find("Content-Type:", filenameEnd);
	if (contentTypePos == std::string::npos) {
		std::cerr << "❌ No Content-Type header found for file part.\n";
		return;
	}

	size_t contentStartMarker = request.find("\r\n\r\n", contentTypePos);
	if (contentStartMarker == std::string::npos) {
		std::cerr << "❌ Could not find blank line after Content-Type.\n";
		return;
	}

	// The actual file content starts immediately after the \r\n\r\n
	size_t contentStart = contentStartMarker + 4;

	// Now we need to find the end of the file data, which is marked by a boundary
	// First, extract the boundary from the Content-Type header
	size_t boundaryPos = request.find("boundary=");
	if (boundaryPos == std::string::npos) {
		std::cerr << "❌ No boundary parameter found.\n";
		return;
	}

	// Extract the boundary string, handling both quoted and unquoted formats
	std::string rawBoundary;
	size_t boundaryStart = boundaryPos + 9; // "boundary=" length

	if (request[boundaryStart] == '"') {
		// Quoted boundary
		boundaryStart++;
		size_t boundaryEnd = request.find("\"", boundaryStart);
		if (boundaryEnd == std::string::npos) {
			std::cerr << "❌ Invalid quoted boundary format.\n";
			return;
		}
		rawBoundary = request.substr(boundaryStart, boundaryEnd - boundaryStart);
	} else {
		// Unquoted boundary
		size_t boundaryEnd = request.find_first_of(" \r\n;", boundaryStart);
		if (boundaryEnd == std::string::npos) {
			// If no terminator, use the rest of the line
			rawBoundary = request.substr(boundaryStart);
		} else {
			rawBoundary = request.substr(boundaryStart, boundaryEnd - boundaryStart);
		}
	}

	if (rawBoundary.empty()) {
		std::cerr << "❌ Empty boundary string.\n";
		return;
	}

	// The boundary in the content will be prefixed with "--"
	std::string boundary = "--" + rawBoundary;
	std::cout << "Boundary: " << boundary << std::endl;

	// Find the first occurrence of the boundary after the content start
	size_t contentEnd = request.find(boundary, contentStart);
	if (contentEnd == std::string::npos) {
		std::cerr << "❌ Could not find closing boundary.\n";
		return;
	}

	// Check if there's a CRLF before the boundary and adjust for it
	if (contentEnd >= 2) {
		if (request[contentEnd-1] == '\n' && request[contentEnd-2] == '\r') {
			contentEnd -= 2; // Adjust to remove CRLF before boundary
		}
	}

	// Extract the file content
	if (contentEnd <= contentStart) {
		std::cerr << "❌ Invalid content boundaries (end <= start).\n";
		return;
	}

	// Use c_str() and careful binary handling to preserve the exact bytes
	size_t contentLength = contentEnd - contentStart;
	std::cout << "Content length: " << contentLength << " bytes" << std::endl;

	const char* fileData = request.c_str() + contentStart;

	// Create a temporary file for debugging/comparison
	std::string tempPath = "/tmp/upload_debug_" + filename;
	std::ofstream tempFile(tempPath.c_str(), std::ios::binary);
	if (tempFile.is_open()) {
		tempFile.write(fileData, contentLength);
		tempFile.close();
		std::cout << "Debug file saved to " << tempPath << std::endl;
	}

	// Save to final destination
	std::string filePath = config.root + "/upload/" + filename;
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "❌ Could not open destination file: " << filePath << "\n";
		return;
	}

	outFile.write(fileData, contentLength);
	outFile.close();

	if (outFile.fail()) {
		std::cerr << "❌ Error writing file.\n";
		return;
	}

	std::cout << "✅ File successfully saved: " << filePath << " (" << contentLength << " bytes)" << std::endl;

	// Send success response with proper charset
	// Create a response with a complete HTML5 document
	std::string responseBody = "<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"    <meta charset=\"UTF-8\">\n"
		"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"    <title>Upload Result</title>\n"
		"    <style>\n"
		"        body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
		"        h1 { color: #4CAF50; }\n"
		"        .button { display: inline-block; background: #4CAF50; color: white; padding: 10px 20px; text-decoration: none; border-radius: 4px; margin-top: 20px; }\n"
		"    </style>\n"
		"</head>\n"
		"<body>\n"
		"    <h1>Upload successful! 🎉</h1>\n"
		"    <p>Your file has been uploaded successfully.</p>\n"
		"    <a href=\"/upload.html\" class=\"button\">Upload another file</a>\n"
		"</body>\n"
		"</html>";

	// Get the exact byte size of the response body
	size_t bodySize = responseBody.size();

	// Debug the body size calculation
	std::cout << "Response body (length=" << bodySize << " bytes):" << std::endl;

	// Build the response with accurate Content-Length
	std::ostringstream oss;
	oss << "HTTP/1.1 200 OK\r\n";
	oss << "Content-Type: text/html; charset=UTF-8\r\n";
	oss << "Content-Length: " << bodySize << "\r\n";
	oss << "Connection: close\r\n"; // Explicitly close the connection
	oss << "\r\n";
	oss << responseBody;

	std::string success = oss.str();
	// Send with error checking
	ssize_t sent = send(client_fd, success.c_str(), success.size(), 0);
	if (sent != (ssize_t)success.size()) {
		std::cerr << "❌ Failed to send complete response: " << sent << " of " << success.size() << " bytes" << std::endl;
	}
}