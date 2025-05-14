/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testDisconnectNoFileSize.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:50:11 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/14 16:28:39 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

int main() {
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
	//open directory of test files or use the file path provided
	std::string name = "./test/samples/test.txt";

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

	std::cout << "🚫 Disconnecting before sending file size...\n";
	close(clientSocket);
	return 0;
}