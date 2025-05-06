#ifndef CONFIGPARSER_HPP
#define	CONFIGPARSER_HPP

#include "WebServ.hpp"
#include "ServerConfig.hpp"
#include <vector>


class ConfigParser {
public:
  ConfigParser();
  ~ConfigParser();
  void	parseFile(const std::string& path);
  const std::vector<ServerConfig>& getServers() const;

private:
  std::vector<ServerConfig> servers;
  void	parseServerBlock(std::ifstream& file, ServerConfig& server);
  void	parseLocationBlock(std::ifstream& file, LocationConfig& location);

};

#endif