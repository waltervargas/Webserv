/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:27 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/18 12:47:27 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WebServ.hpp"
#include <vector>


class ConfigParser {
public:
  ConfigParser();
  ~ConfigParser();
  const std::vector<ServerConfig>& getServers() const;
  void	parseFile(const std::string& path);
  void	parseServerBlock(std::ifstream& file, ServerConfig& server);
  void	parseLocationBlock(std::ifstream& file, LocationConfig& location);
  void  parseServerDirective(ServerConfig& server, const std::string& key, const std::string& value);
  void	parseLocationDirective(LocationConfig& location, const std::string& key, const std::string& value);
  void	applyInheritance(LocationConfig& location, const ServerConfig& server);
  void	error(const std::string& msg) const;
  void  print() const;

  private:
  std::vector<ServerConfig> servers;

};
