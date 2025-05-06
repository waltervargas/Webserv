#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <cctype> //for std::tolower
#include <dirent.h> //to allow access to file directory
#include "include/WebServ.hpp"
#include "include/ConfigParser.hpp"
#include "include/ServerSocket.hpp"

std::string toLower(const std::string& str) {
	std::string result;
	for (size_t i = 0; i < str.size(); ++i)
		result += std::tolower(str[i]);
	return result;
}

bool isFile(const std::string& name) {
	std::string	lowerName = toLower(name);
	const char* extensions[] = {".jpg", ".jpeg", ".png", ".heic", ".txt"};
	int count = sizeof(extensions) / sizeof(extensions[0]);
	for (int i = 0; i < count; i++) {
		const char* ext = extensions[i];
		size_t	extLen = std::strlen(ext);
		if (lowerName.size() >= extLen && lowerName.compare(lowerName.size() - extLen, extLen, ext) == 0)
			return true;
	}
	return false;
}

int main() {

	ConfigParser	parser;

	parser.parseFile("default.config");
	const std::vector<ServerConfig>& servers = parser.getServers();
	if (!servers.empty()) {
		std::cerr << "❌ Problem retrieving servers in test client\n";
		return 1;
	}

	//initialise the first server
	int port = servers[0].port;

	std::cout << "Starting server on port" << port << std::endl;
	//creating socket
	int	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	//specify address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;	
	serverAddress.sin_port = htons(port); 
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	//send connection request
	connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	//open directory of test files
	DIR* dir = opendir("./test");
	if (!dir) {
		std::cerr << "Failed to open directory\n";
		return 1;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string fileName = entry->d_name;
		if (fileName == "." || fileName == "..")
			continue;
//f		std::cout << "Checking file: " << fileName << std::endl;
		if (!isFile(fileName))
			continue;
		std::string filePath = "./test/" + fileName;

		//send filename length
		uint8_t	nameLen = fileName.size();
		if (send(clientSocket, &nameLen, 1, 0) == -1){
			std::cerr << "Failed to send filename length\n";
			return 1;
		}

		//send filename
		if (send(clientSocket, fileName.c_str(), nameLen, 0) == -1) {
			std::cerr << "Failed to send filename\n";
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
		//gcount() Returns the number of characters extracted by the last 
		//unformatted input operation performed on the object.
		send(clientSocket, buffer, file.gcount(), 0);
		file.close();
		std::cout << "Sent: " << fileName << std::endl;
	}
	closedir(dir);
	close(clientSocket);
	return 0;
}