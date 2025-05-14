/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testDisconnectMidSend.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:50:11 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/14 03:42:00 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

int main() {
	const std::string fileName = "test.txt";
	std::string filePath = std::string("./test_samples/") + fileName;
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file) {
		std::cerr << "❌ Failed to open file.\n";
		return 1;
	}
	
	//creating socket
	int	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	//specify address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8081);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

	//send connection request
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "❌ Client failed to connect\n";
		return 1;
	}
	std::cout << "✅ Connected to server.\n";

	//send filename length and name
	uint8_t	len = fileName.size();
	send(clientSocket, &len, 1, 0);
	send(clientSocket, fileName.c_str(), len, 0);

	//get file size
	file.seekg(0, std::ios::end);
	//tellg() Returns the position of the current character in the input stream.
	uint32_t	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	send(clientSocket, &fileSize, sizeof(fileSize), 0);

	//send half the data
	char	buffer[1024];
	uint32_t	half = fileSize / 2;
	uint32_t	sent = 0;

	while (sent < half && file.read(buffer, sizeof(buffer))) {
		size_t chunk = file.gcount();
		if (sent + chunk > half) chunk = half - sent;
		send(clientSocket, buffer, chunk, 0);
		sent += chunk;
	}
	
	std::cout << "🚫 Disconnecting halfway through file...\n";
	close(clientSocket);
	return 0;
}