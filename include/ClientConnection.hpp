#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

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

#endif