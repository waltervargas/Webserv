/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:13:21 by kellen            #+#    #+#             */
/*   Updated: 2025/05/18 17:17:01 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Response.hpp"
#include <sstream>

/*
* Constructor — currently does nothing, but good to have.
*/
Response::Response() {}

/*
* This function builds a full HTTP 200 OK response.
*/
std::string Response::build200(const std::string& body, const std::string& contentType) {
	std::ostringstream response;
	response << buildHeader(200, "OK", body.size(), contentType);
	response << body;
	return response.str();
}

/*
* This function builds a full HTTP 404 Not Found response.
*/
std::string Response::build404() {
	std::string body = "<h1>404 - Not Found 💔</h1>";
	return build200(body); // reuse 200 builder with a 404 message
}

/*
* This function builds the HTTP header for a given status and content.
*/
std::string Response::buildHeader(int statusCode, const std::string& statusText, size_t contentLength, const std::string& contentType) {
	std::ostringstream header;
	header << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
	header << "Content-Length: " << contentLength << "\r\n";
	header << "Content-Type: " << contentType << "; charset=utf-8\r\n";
	header << "\r\n";
	return header.str();
}

/*
 * This function returns the appropriate Content-Type for a given file path.
 */
std::string Response::getContentType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot);

	if (ext == ".html") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "application/javascript";
	if (ext == ".png")  return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".ico")  return "image/x-icon";
	if (ext == ".svg")  return "image/svg+xml";
	if (ext == ".txt")  return "text/plain";
	if (ext == ".json") return "application/json";

	return "application/octet-stream";
}
