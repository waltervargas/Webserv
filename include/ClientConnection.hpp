/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/12 13:49:43 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class ClientConnection {
  private:
    int         _fd;
    std::string _message;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    bool        receiveMessage();
    bool        sendMessage();
    std::string	getMessage() const;
    int         getFd() const;
    void        closeConnection();
};
