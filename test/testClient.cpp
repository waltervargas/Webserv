/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testClient.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:17:48 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/02 15:17:04 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <cctype> //for std::tolower
#include <dirent.h> //to allow access to file directory
#include <signal.h>
#include "../include/ConfigParser.hpp"
#include <arpa/inet.h>

std::string toLower(const std::string& str) {
	std::string result;
	for (size_t i = 0; i < str.size(); ++i)
		result += std::tolower(str[i]);
	return result;
}

bool isFile(const std::string& name) {
	std::string	lowerName = toLower(name);
	const char* extensions[] = {".jpg", ".jpeg", ".png", ".heic", ".txt", ".pdf", ".cpp", ".doc"};
	int count = sizeof(extensions) / sizeof(extensions[0]);
	for (int i = 0; i < count; i++) {
		const char* ext = extensions[i];
		size_t	extLen = std::strlen(ext);
		if (lowerName.size() >= extLen && lowerName.compare(lowerName.size() - extLen, extLen, ext) == 0)
			return true;
	}
	return false;
}

std::string createUniqueFileName(const std::string& base, const std::string& outputDir) {
	std::string name = base;
	int counter = 1;
	while (true) {
		std::string	fullPath = outputDir + "/" + name;
		if (access(fullPath.c_str(), F_OK) != 0) //if file doesn't exist, use this name
			break;

		size_t dot = base.find_last_of(".");
		std::string nameOnly = base.substr(0, dot);
		std::string ext = base.substr(dot);
		std::ostringstream oss;
		oss << nameOnly << "_" << counter << ext;
		name = oss.str();
		counter++;
	}
	return name;
}

int main(int ac, char **av) {

	ConfigParser	parser;
	signal(SIGPIPE, SIG_IGN);

	parser.parseFile("conf/default.conf");
	const std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Problem retrieving servers in test client\n";
		return 1;
	}

	//initialise the first server
	int port = servers[0].port;

	std::cout << "Starting server on port: " << port << std::endl;
	//creating socket
	int	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	//specify address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

	//send connection request
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "❌ Client failed to connect\n";
		return 1;
	}
	std::cout << "✅ Connected to server.\n";
	//open directory of test files or use the file path provided
	std::string inputDir = "test/samples/";
	if (ac == 2)
		inputDir = av[1];
	DIR* dir = opendir(inputDir.c_str());
	if (!dir) {
		std::cerr << "Failed to open directory: " << inputDir << std::endl;
		return 1;
	}
	std::set<std::string> usedNames;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string original = entry->d_name;
		if (original == "." || original == "..")
			continue;
		//f		std::cout << "Checking file: " << fileName << std::endl;
		if (!isFile(original))
			continue;
		std::string filePath = inputDir + "/" + original;
		
		std::string fileName = createUniqueFileName(original,"test/output");
		//send filename length
		uint8_t	nameLen = fileName.size();
		if (send(clientSocket, &nameLen, 1, 0) <= 0){
			std::cerr << "Failed to send filename length\n";
			return 1;
		}

		//send filename
		if (send(clientSocket, fileName.c_str(), nameLen, 0) == -1) {
			std::cerr << "Failed to send filename: " << strerror(errno) << "\n";
			return 1;
		}

		std::ifstream	file(filePath.c_str(), std::ios::binary);
		if (!file) {
			std::cerr << "Cannot open input\n";
			return 1;
		}
		file.seekg(0, std::ios::end);
		//tellg() Returns the position of the current character in the input stream.
		uint32_t	fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		if (send(clientSocket, &fileSize, sizeof(fileSize), 0) == -1) {
			std::cerr << "Failed to send file size\n";
			return 1;
		}

		char buffer[1024];
		while (file.read(buffer, sizeof(buffer))) {
			send(clientSocket, buffer, sizeof(buffer), 0);
		}
		//std::cout << "Sent chunk of " << file.gcount() << " bytes\n";
		//gcount() Returns the number of characters extracted by the last 
		//unformatted input operation performed on the object.
		send(clientSocket, buffer, file.gcount(), 0);
		//std::cout << "Sent chunk of 2" << file.gcount() << " bytes\n";
		file.close();
		//std::cout << "Sent: " << fileName << std::endl;
		usleep(100000); //pause for 0.1 seconds
	}
	closedir(dir);
	close(clientSocket);
	return 0;
}