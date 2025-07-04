/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/04 16:12:46 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <string>

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
    int         recvFullRequest(int client_fd, const ServerConfig& config);
};

#endif // CLIENTCONNECTION_HPP
