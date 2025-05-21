/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:09:45 by kellen            #+#    #+#             */
/*   Updated: 2025/05/21 18:04:12 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpStatus.hpp"
#include <string>

/*
The Response class builds the HTTP response to be sent to the client.
 */
class Response {
	public:
		Response();
		std::string getContentType(const std::string& path);
		static std::string buildHeader(int statusCode, size_t contentLength, const std::string& contentType);
		static std::string build(int statusCode, const std::string& body, const std::string& contentType);
};
