/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testWrongLengthFile.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:50:11 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/14 03:14:21 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

int main() {

	std::string name = "malformedText.txt";
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

	uint8_t	len = name.length();
	if (send(clientSocket, &len, 1, 0) == -1){
		std::cerr << "Failed to send test.txt length\n";
		return 1;
	}

	//send filename
	if (send(clientSocket, name.c_str(), len, 0) == -1) {
		std::cerr << "Failed to send filename\n";
		return 1;
	}

	//send wrong file size (2 GB), this wraps around and fileSize becomes 0
	//as uint32_t can only store up to 4,294,967,295
	uint32_t	fileSize = 2U * 1024U * 1024U * 1024U;
//	uint32_t fileSize = 0x7FFFFFFF; //test for large overflow
	send(clientSocket, &fileSize, sizeof(fileSize), 0);

	std::cout << "💣 Sent bad file size...\n";
	close(clientSocket);
	return 0;
}