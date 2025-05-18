/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:52 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/18 11:44:54 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"
#include <iostream>

void	ServerConfig::print() const {
	std::cout << "\n======================" << std::endl;
	std::cout << "listen: " << port << std::endl;
	std::cout << "host: " << host << std::endl;
	std::cout << "server_name: " << server_name << std::endl;
	std::cout << "root: " << root << std::endl;
	std::cout << "index: " << index << std::endl;
	std::cout << "client_max_body_size: " << client_max_body_size << std::endl;

	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
		std::cout << "error_page " << it->first << " => " << it->second << std::endl;
	
	for (size_t i = 0; i < locations.size(); ++i)
		locations[i].print();

	if (!raw.empty()) {
		std::cout << "\nSERVER RAW DIRECTIVES:\n";
		for (std::map<std::string, std::string>::const_iterator it = raw.begin(); it != raw.end(); ++it)
			std::cout << "    " << it->first << ": " << it->second << std::endl;
	}
}

void	applyDefaults(ServerConfig& server) {
	if (server.port == 0)
		server.port = 80;
	if (server.host.empty())
		server.host = "0.0.0.0";
	if (server.root.empty())
		server.root = "/var/www.html";
	if (server.index.empty())
		server.index = "index.html";
	if (server.server_name.empty())
		server.server_name = "default_server";
	if (server.client_max_body_size == 0)
		server.client_max_body_size = 1000000;
}
