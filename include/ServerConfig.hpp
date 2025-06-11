/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:17 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/27 11:06:44 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>

struct	ServerConfig {
		//raw is for testing, ensure we process everything (remove before finishing)
	std::map<std::string, std::string> raw; //stores unprocessed directives
	std::vector<int>			ports; //converted and valid ports pulled from listen_entries
	std::vector<std::string>	listen_entries; //read initial ports as string
	std::string					host; // IP or host name
	std::string					server_name;
	std::string					root; //root directory for requests
	std::string					index; //default directory if no URI provided
	long						client_max_body_size;
	std::map<int, std::string>	error_pages; //error code and path
	std::vector<LocationConfig>	locations; //location blocks
	
	ServerConfig();

	void	print() const;
	void	loadErrorPages();
};

void		applyDefaults(ServerConfig& server);