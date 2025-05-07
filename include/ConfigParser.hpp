#ifndef CONFIGPARSER_HPP
#define	CONFIGPARSER_HPP

#include "WebServ.hpp"
#include "ServerConfig.hpp"
#include <vector>


class ConfigParser {
public:
  ConfigParser();
  ~ConfigParser();
  const std::vector<ServerConfig>& getServers() const;
  void	parseFile(const std::string& path);
  void	parseServerBlock(std::ifstream& file, ServerConfig& server);
  void	parseLocationBlock(std::ifstream& file, LocationConfig& location);
  
  private:
  std::vector<ServerConfig> servers;

};

#endif