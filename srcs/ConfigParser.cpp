#include "../include/WebServ.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

void	ConfigParser::parseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("❌ Error: Could not open config file: " + path);
	}
	std::string line;
	while (std::getline(file, line)) {
		std::cout << "Parsing line: " << line << std::endl;
		trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		std::istringstream iss(line);
		std::string key, next;
		iss >> key >> next;
		if (key == "server" && next == "{") {
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
		if (line == "{") depth++;
		if (line == "}") {
			depth--;
			if (depth == 0) return;
			continue;
		}

		if (line.find("location") == 0 && line.find("{") != std::string::npos) {
			LocationConfig	location;
			size_t	start = line.find("location") + 8;
			size_t	end = line.find("{");
			std::string	path = line.substr(start, end - start);
			trim(path);
			location.path = path;
			parseLocationBlock(file, location);
			server.locations.push_back(location);
		}
		else {
			std::istringstream iss(line);
			std::string key, value;
			iss >> key >> value;
			if (!value.empty() && value[value.size() - 1] == ';')
				value = value.substr(0, value.size() - 1);


			// 🌈 Parse the key-value pairs
			// for now, we only handle listen, host, root, and index
			// later we will add more options
			// like error pages, cgi, etc.
			if (key == "listen") server.port = std::atoi(value.c_str());
			else if (key == "host") server.host = value;
			else if (key == "root") server.root = value;
			else if (key == "index") server.index = value;
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
		if (line == "{") depth++;
		if (line == "}") {
			depth--;
			if (depth == 0)
				return;
			continue;
		}
		std::istringstream iss(line);
		std::string key, value;
		iss >> key >> value;
		if (!value.empty() && value[value.size() - 1] == ';')
		value = value.substr(0, value.size() - 1);


		// 🌈 Parse the key-value pairs
		// for now, we only handle listen, host, root, and index
		// later we will add more options
		// like error pages, cgi, etc.

		if (key == "root") location.root = value;
		else if (key == "index") location.index = value;
	}
}

const	std::vector<ServerConfig>& ConfigParser::getServers() const {
	return servers;
}