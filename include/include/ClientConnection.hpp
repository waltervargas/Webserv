/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/29 13:59:13 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "../include/ServerConfig.hpp"

enum ClientState {
  READING_HEADERS,
  READING_BODY,
  REQUEST_COMPLETE
};

class ClientConnection {
  private:
    int               _fd;
    std::vector<char> _buffer;
  
  public:
    ClientConnection(int fd);
    ~ClientConnection();

    std::string	getRawRequest() const;
    int         getFd() const;
    void        closeConnection();
    bool        isRequestComplete() const;
    void        recvFullRequest(int client_fd, const ServerConfig& config);
};
