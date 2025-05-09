/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:17 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/09 14:09:43 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>

struct	ServerConfig {
	int							port;
	std::string					host;
	std::string					server_name;
	std::string					root;
	std::string					index;
	size_t						client_max_body_size;
	std::map<int, std::string>	error_pages; //error code and path
	std::vector<LocationConfig>	locations;

	void	print() const;
};
