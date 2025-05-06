#ifndef	SERVERCONFIG_HPP
#define	SERVERCONFIG_HPP

#include <string>
#include <vector>
#include "WebServ.hpp"
#include "LocationConfig.hpp"

struct	ServerConfig {
	int							port;
	std::string					host;
	std::string					root;
	std::string					index;
	std::vector<LocationConfig>	locations;
};

#endif