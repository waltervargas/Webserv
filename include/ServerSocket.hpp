#ifndef	SERVERSOCKET_HPP
#define	SERVERSOCKET_HPP

#include <cstring>
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <cerrno> //errno
#include <stdexcept> //std::runtime_error
#include <netinet/in.h>
#include <sys/socket.h> //internet protocol family

#define _XOPEN_SOURCE_EXTENDED 1

//external helpers
int		safe_socket(int domain, int type, int protocol);
bool	safe_bind(int fd, sockaddr_in & addr);
bool	safe_listen(int socket, int backlog);

class ServerSocket {
  private:
    int		_fd;
  public:
    ServerSocket();
    ~ServerSocket();

    bool	init(int port);
    int		acceptClient();
    void	closeSocket();
    int		getFD();

};

#endif


