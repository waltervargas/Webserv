#ifndef	SERVERSOCKET_HPP
#define	SERVERSOCKET_HPP

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

#endif


