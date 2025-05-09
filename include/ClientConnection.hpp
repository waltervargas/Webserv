/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/09 13:53:31 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WebServ.hpp"
#include <string>

class ClientConnection {
  private:
    int         _fd;
    std::string _message;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    bool        receiveMessage();
    std::string	getMessage() const;
    int         getFd() const;
    void        closeConnection();
};
