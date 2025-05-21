/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/21 18:23:14 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "../include/ServerConfig.hpp"

class ClientConnection {
  private:
    int         _fd;
    std::string _message;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    bool        sendMessage();
    std::string	getMessage() const;
    int         getFd() const;
    void        closeConnection();
    std::string recvFullRequest(int client_fd, const ServerConfig& config);
};
