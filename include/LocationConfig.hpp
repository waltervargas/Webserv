/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:22 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/20 14:29:40 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <map>
#include <vector>

struct	LocationConfig {
	//raw is for testing, ensure we process everything (remove before finishing)
	std::map<std::string, std::string> raw;//stores unprocessed directives
	std::string	path;// URL path (e.g. /updload)
	std::string	root; // root directory for this location
	std::vector<std::string> methods; // allowed http methods
	std::string	index; // default file to serve
	std::string	redirect; //URL to redirect if set
	std::string	upload_path; //where uploaded files are stored
	std::map<std::string, std::string> cgi_paths; // map ext -> CGI binary
	bool	autoindex; //enable directory listing
	bool	root_set; //track override
	bool	index_set; //track override

	LocationConfig();
	void	print() const;
};
