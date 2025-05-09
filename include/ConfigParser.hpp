/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:27 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/09 14:06:25 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include <vector>


class ConfigParser {
public:
  ConfigParser();
  ~ConfigParser();
  const std::vector<ServerConfig>& getServers() const;
  void	parseFile(const std::string& path);
  void	parseServerBlock(std::ifstream& file, ServerConfig& server);
  void	parseLocationBlock(std::ifstream& file, LocationConfig& location);
  void  print() const;

  private:
  std::vector<ServerConfig> servers;

};
