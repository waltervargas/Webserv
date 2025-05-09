/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:08 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/09 13:53:09 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WebServ.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

//external helpers
int		safe_socket(int domain, int type, int protocol);
bool	safe_bind(int fd, sockaddr_in & addr);
bool	safe_listen(int socket, int backlog);

class ServerSocket {
  private:
    int		_fd;
    int   _port;
  public:
    ServerSocket();
    ~ServerSocket();

    bool	init(int port);
    int		acceptClient();
    void	closeSocket();
    int		getFD();
    int   getPort();
};



