#include "../include/WebServ.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}


static bool	parseKeyValue(const std::string& line, std::string& key, std::string& value) {
	std::istringstream iss(line);
	if (!(iss >> key))
		return false;
	std::getline(iss, value);
	trim(value);
	if (!value.empty() && value[value.size() - 1] == ';')
		value = value.substr(0, value.size() - 1);
	trim(value);
	return !value.empty(); //only return true if a value was found/parsed
}

std::vector<std::string> line_splitter(const std::string& line) {
	std::vector<std::string>	result;
	std::istringstream iss(line);
	std::string	token;
	while (iss >> token)
		result.push_back(token);
	return result;
}

void	ConfigParser::parseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("❌ Error: Could not open config file: " + path);
	}
	std::string line;
	while (std::getline(file, line)) {
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		if (line.find("server") == 0) {
			std::istringstream iss(line);
			std::string	type, brace;
			iss >> type >> brace;

			if (type == "server" && brace == "{") {
				ServerConfig	server;
				parseServerBlock(file, server);
				servers.push_back(server);
				continue;
			}
			else
				std::cerr << "⚠️ couldn't read server block\n";
		}	
	}
}

void ConfigParser::parseServerBlock(std::ifstream& file, ServerConfig& server) {
	std::string	line;
	int depth = 1;
	while (std::getline(file, line)) {
		trim(line);
		// ❌ Skip comments and empty lines
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "{") {
			depth++;
			continue;
		}
		if (line == "}") {
			depth--;
			if (depth == 0) return;
			continue;
		}
		if (line.find("location") == 0) {
			std::istringstream iss(line);
			std::string	type, path, brace;
			iss >> type >> path >> brace;

			if (type == "location" && brace == "{") {
				LocationConfig	location;
				location.path = path; //save URL path for location block (upload etc)
				std::cout << "\nLOCATION " << location.path << ": \n";
				parseLocationBlock(file, location);
				server.locations.push_back(location);
				continue;
			}
			else
				std::cerr << "⚠️ couldn't read location block\n";
		}	
		std::string key, value;
		if (parseKeyValue(line, key, value)) {
			if (value.empty()) {
				std::cerr << "⚠️ value is empty for " << key << "skipping line\n";
				continue;
			}
			if (key == "listen") {
				server.port = std::atoi(value.c_str());
			}
			else if (key == "host") {
				server.host = value;
//				std::cout << "host: " << server.host << std::endl;
			}
			else if (key == "server_name") {
				server.server_name = value; 
//				std::cout << "server_name: " << server.server_name << std::endl;
			}
			else if (key == "root") {
				server.root = value;
//				std::cout << "root: " << server.root << std::endl;
			}
			else if (key == "index") {
				server.index = value;
//				std::cout << "index: " << server.index << std::endl;
			}
			else if (key == "client_max_body_size") {
				server.client_max_body_size = std::atol(value.c_str());
//				std::cout << "client_max_body_size: " << server.client_max_body_size << std::endl;
			}
			else if (key == "error_page") {
				std::istringstream iss(value);
				int	code;
				std::string	path;
				if (iss >> code >> path) {
					server.error_pages[code] = path;
//					std::cout << "error page " << code << " set to " << path << std::endl;
				}
				else
					std::cerr << "⚠️ Couldn't read error page\n";
			}
		}
	}
}

void ConfigParser::parseLocationBlock(std::ifstream& file, LocationConfig& location) {
	std::string	line;
	int depth = 1;

	while (std::getline(file, line)) {
		trim(line);
		// ❌ Skip comments and empty lines
		if (line.empty() || line[0] == '#') continue;
		if (line == "{") {
			depth++;
			continue;
		}
		if (line == "}") {
			depth--;
			if (depth == 0)
				return;
			continue;
		}
		std::string key, value;
		if (parseKeyValue(line, key, value)) {
			if (key == "root") {
				location.root = value;
//				std::cout << "root: " << location.root << std::endl;
			}
			else if (key == "index") {
				location.index = value;
//				std::cout << "index: " << location.index << std::endl;
			}
			else if (key == "redirect") {
				location.redirect = value;
//				std::cout << "redirect: " << location.redirect << std::endl;
			}
			else if (key == "autoindex") {
				if (value == "on")
					location.autoindex = true;
//				std::cout << "autoindex: " << location.autoindex << std::endl;
			}
			else if (key == "allowed_methods" || key == "methods") {
				location.methods = line_splitter(value);
//				std::cout << "methods: ";
//				for (size_t i = 0; i < location.methods.size(); i++)
//					std::cout << location.methods[i] << " ";
//				std::cout << std::endl;
			}
			else if (key == "upload_path") {
				location.upload_path = value;
//				std::cout << "upload_path: " << location.upload_path << std::endl;
			}
			else if (key == "cgi_path") {
				location.cgi_paths["default"] = value;
//				std::cout << "Default CGI path set: " << location.cgi_paths["default"] << std::endl;
			}
			else if (key == "cgi") {
				std::vector<std::string> parts = line_splitter(value);
				if (parts.size() == 2) {
					location.cgi_paths[parts[0]] = parts[1];
//					std::cout << "CGI extension: " << parts[0] << std::endl;
				}
				else
					std::cerr << "⚠️ Couldn't read cgi directives\n";
			}
		}
	}
}

const	std::vector<ServerConfig>& ConfigParser::getServers() const {
	return servers;
}

void	ConfigParser::print() const {
	for (size_t i = 0; i < servers.size(); i++) {
		std::cout << "\n SERVER " << i + 1 << std::endl;
		servers[i].print();
	}
}

