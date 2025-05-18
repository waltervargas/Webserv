/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:09:45 by kellen            #+#    #+#             */
/*   Updated: 2025/05/18 17:18:11 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

/*
 * The Response class builds the HTTP response to be sent to the client.
 */
class Response {
	public:
		Response();
		std::string build200(const std::string& body, const std::string& contentType = "text/html");
		std::string build404();
		std::string getContentType(const std::string& path);

	private:
		std::string buildHeader(int statusCode, const std::string& statusText, size_t contentLength, const std::string& contentType);
};





/* Response Response::makeErrorResponse(int statusCode, const Config &config) {
	// TODO: Use custom error pages from config

	std::string body = "<html><head><title>Error " + std::to_string(statusCode) + "</title></head>"
					"<body><h1>Error " + std::to_string(statusCode) + "</h1>";

	switch (statusCode) {
		case 400:
			body += "<p>Bad Request</p>";
			break;
		case 403:
			body += "<p>Forbidden</p>";
			break;
		case 404:
			body += "<p>Not Found</p>";
			break;
		case 405:
			body += "<p>Method Not Allowed</p>";
			break;
		case 500:
			body += "<p>Internal Server Error</p>";
			break;
		case 501:
			body += "<p>Not Implemented</p>";
			break;
		default:
			body += "<p>Unknown Error</p>";
			break;
	}

	body += "</body></html>";

	Response response(statusCode, body);
	response.setHeader("Content-Type", "text/html");

	return response;
} */