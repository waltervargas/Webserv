/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/25 15:23:47 by kbolon           ###   ########.fr       */
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
    ClientState       _state;
    std::vector<char> _buffer;
    size_t            _contentLength;
    size_t            _bodyStart;
    bool              _knownContentLength;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    std::string	getRawRequest() const;
    int         getFd() const;
    void        closeConnection();
    bool        readRequest(const ServerConfig& config);
};
