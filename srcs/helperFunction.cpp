
#include "../include/WebServ.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/LocationConfig.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

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

/*void	parseBlock(std::ifstream& file, const std::string& type, ServerConfig& server) {
	if (type == "server") {
		parseServerBlock(file, server);
		servers.puch_back(server);
	}
	else if (type == "location") {
		LocationConfig location;
		parseLocationBlock(file, location);
		server.locations.push_back(location);
	}
}*/