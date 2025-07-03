/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:22:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/03 16:05:37 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <string>
#include <map>

class HttpStatus {
  public:
    static const std::map<int, std::string>& statusMessages();
	static std::string getStatusMessages(int code);
};

#endif // HTTPSTATUS_HPP