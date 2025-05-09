#include "../include/ServerConfig.hpp"
#include "../include/LocationConfig.hpp"
#include <iostream>

void	ServerConfig::print() const {
	std::cout << "\nSERVER:\n";
	std::cout << "listen: " << port << std::endl;
	std::cout << "host: " << host << std::endl;
	std::cout << "server_name: " << server_name << std::endl;
	std::cout << "root: " << root << std::endl;
	std::cout << "index: " << index << std::endl;
	std::cout << "client_max_body_size: " << client_max_body_size << std::endl;

	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
		std::cout << "  error_page " << it->first << " => " << it->second << std::endl;
	
	for (size_t i = 0; i < locations.size(); ++i)
		locations[i].print();
}