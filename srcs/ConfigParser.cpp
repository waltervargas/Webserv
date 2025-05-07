#include "../include/WebServ.hpp"

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

static bool	parseType(const std::string& line, const std::string& expected_type) {
	std::istringstream iss(line);
	std::string	type, next, character;
	if (!(iss >> type))//checks if server or location
		return false;
	if (expected_type == "server" && type == "server" && iss >> character && character == "{") {
		return true;
	}
	if (expected_type == "location" && type == "location" && iss >> character && character == "/") {
		return true;
	}
	return false;
}

void	ConfigParser::parseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("❌ Error: Could not open config file: " + path);
	}
	std::string line;
	while (std::getline(file, line)) {
//		std::cout << "Parsing line: " << line << std::endl;
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		std::cout << "in Parse File\n";
		if (parseType(line, "server")) {
//			std::cout << "SERVER BLOCK FOUND\n";
			ServerConfig	server;
			parseServerBlock(file, server);
			servers.push_back(server);
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

		if (parseType(line, "location")) {
			LocationConfig	location;
			parseLocationBlock(file, location);
			server.locations.push_back(location);
			continue;
		}
		std::string key, value;
		if (parseKeyValue(line, key, value)) {
			if (value.empty()) {
				std::cerr << "⚠️ value is empty for " << key << "skipping line\n";
				continue;
			}
			if (key == "listen") {
				server.port = std::atoi(value.c_str());
				std::cout << "Server block listen: " << server.port << std::endl;
			}
			else if (key == "host") {
				server.host = value; 
				std::cout << "Server block host: " << server.host << std::endl;
			}
			else if (key == "root") {
				server.root = value;
				std::cout << "Server block root: " << server.root << std::endl;
			}
			else if (key == "index") {
				server.index = value;
				std::cout << "Server block index: " << server.index << std::endl;
			}
			else if (key == "error_page")
			std::cout << "Server block error page\n";
			}
		}
	}


void ConfigParser::parseLocationBlock(std::ifstream& file, LocationConfig& location) {
	std::string	line;
	int depth = 1;
	std::cout << "LOCATION BLOCK FOUND\n";

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
				std::cout << "Location block root: " << location.root << std::endl;
			}
			else if (key == "index") {
				location.index = value;
				std::cout << "Location block index: " << location.index << std::endl;
			}
		}
	}
}

const	std::vector<ServerConfig>& ConfigParser::getServers() const {
	return servers;
}