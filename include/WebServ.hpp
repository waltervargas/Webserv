/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:27:43 by keramos-          #+#    #+#             */
/*   Updated: 2025/05/20 18:05:22 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include "ClientConnection.hpp"
#include "ServerSocket.hpp"
#include "ServerConfig.hpp"
#include "ConfigParser.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFER_SIZE 4096
#define _XOPEN_SOURCE_EXTENDED 1

class ServerSocket;
class ClientConnection;

std::string	trim(std::string& s);
std::string	cleanValue(std::string s);
std::string getContentType(const std::string& path);
int			safe_socket(int domain, int type, int protocol);
bool		safe_bind(int fd, sockaddr_in & addr);
bool		safe_listen(int socket, int backlog);
void		shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients);
void 		handleUpload(const std::string &request, int client_fd, const ServerConfig &config);
void 		serveStaticFile(std::string path, int client_fd, const ServerConfig &config);
std::string getContentType(const std::string& path);
std::string getInterpreter(const std::string& path, const ServerConfig& config);
void 		handleCgi(const Request req, int fd, const ServerConfig& config, std::string interpreter);
std::string	getErrorPageBody(int code, const ServerConfig& config);
void 		sendHtmlResponse(int fd, int code, const std::string& body);
std::string buildHtmlResponse(int code, const std::string& body);
const char* getStatusMessage(int code);
