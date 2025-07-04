/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiFunctions.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: keramos- <keramos-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 15:38:46 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/04 15:47:12 by keramos-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
find the script/language interpreter by lopping through all config.locations
check that the script extension matches

To Run CGI for testing, once ./webserv is running, in another terminal:
curl http://localhost:8081/cgi-bin/hello.py
*/
std::string getInterpreter(const std::string& path, const ServerConfig& config) {
	for (size_t i = 0; i < config.locations.size(); ++i) {
		const std::map<std::string, std::string>& cgiMap = config.locations[i].cgi_paths;
		for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
			// only match file extension in extension (so no py.backup is used as an example)
			// checks if path is shorter than ext, then checks if end of path string matches the extension exactly (path.compare(startIndex, lengthToCompare, stringToMatch))
			if (path.length() >= it->first.length() && path.compare(path.length() - it->first.length(), it->first.length(), it->first) == 0)
				return it->second;
		}
	}
	return "";
}

const LocationConfig* findMatchingLocation(const std::string& path, const ServerConfig& config) {
	for (size_t i = 0; i < config.locations.size(); ++i) {
		if (path.find(config.locations[i].path) == 0) {
			return &config.locations[i];
		}
	}
	return NULL;
}


bool handleSimpleCGI(int fd, const Request& req, const std::string& path, const ServerConfig& config) {
//	std::cout << "🚀 Starting Simple CGI execution for: " << path << std::endl;

	// Step 1: Find the interpreter for this script
	std::string interpreter = getInterpreter(path, config);
	if (interpreter.empty()) {
		std::cout << "❌ No interpreter found for " << path << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(fd, 500, errorBody);
		return false;
	}

	// Step 2: Build the full path to the script
	std::string scriptPath = config.root + path;

	// Remove query string from script path if present
	size_t queryPos = scriptPath.find('?');
	if (queryPos != std::string::npos) {
		scriptPath = scriptPath.substr(0, queryPos);
	}

	// Step 3: Check if the script file exists
	if (!fileExists(scriptPath)) {
		std::cout << "❌ Script file not found: " << scriptPath << std::endl;
		std::string errorBody = getErrorPageBody(404, config);
		sendHtmlResponse(fd, 404, errorBody);
		return false;
	}

	if (access(scriptPath.c_str(), X_OK) != 0) {
		std::cout << "⚠️ Script may not be executable, but continuing..." << std::endl;
	}

	// Step 4: Execute the script and capture output
	std::string scriptOutput = executeScript(interpreter, scriptPath, req);

	if (scriptOutput.empty()) {
		std::cout << "❌ Script execution failed or returned empty output" << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(fd, 500, errorBody);
		return false;
	}

	// Step 5: Send the script output directly to the client
	if (!safeSend(fd, scriptOutput)) {
		return false;
	}
	return true;
}

// Helper function to execute the script
std::string executeScript(const std::string& interpreter, const std::string& scriptPath, const Request& req) {

	// Create pipes for communication
	int outputPipe[2];
	int inputPipe[2];

	if (pipe(outputPipe) == -1 || pipe(inputPipe) == -1) {
		std::cerr << "❌ Failed to create pipes" << std::endl;
		return "";
	}

	// Fork a new process
	pid_t pid = fork();
	if (pid < 0) {
		std::cerr << "❌ Fork failed" << std::endl;
		close(outputPipe[0]);
		close(outputPipe[1]);
		close(inputPipe[0]);
		close(inputPipe[1]);
		return "";
	}

	if (pid == 0) {
		// Child process: execute the script
//		std::cout << "👶 Child process: executing script" << std::endl;

		// Redirect stdin and stdout
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);

		// Close unused pipe ends
		close(outputPipe[0]);
		close(outputPipe[1]);
		close(inputPipe[0]);
		close(inputPipe[1]);

		// Prepare environment variables
		std::vector<std::string> envStrings;
		envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
		envStrings.push_back("QUERY_STRING=" + req.getQuery());
		envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		envStrings.push_back("CONTENT_LENGTH=" + intToStr(req.getBody().length()));
		envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
		envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envStrings.push_back("SCRIPT_NAME=" + scriptPath);
		if (scriptPath.find(".php") != std::string::npos) {
			envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
			envStrings.push_back("REDIRECT_STATUS=200");
		}

		// HTTP headers
		const std::map<std::string, std::string>& headers = req.getHeaders();
		for (std::map<std::string, std::string>::const_iterator it = headers.begin();
			it != headers.end(); ++it) {

			std::string httpVar = "HTTP_";  // Start with HTTP_ prefix only

			// Transform the header name: hyphens to underscores, all uppercase
			for (size_t i = 0; i < it->first.length(); ++i) {
				char c = it->first[i];
				if (c == '-') {
					httpVar += '_';
				} else {
					httpVar += std::toupper(static_cast<unsigned char>(c));
				}
			}

			std::string envVar = httpVar + "=" + it->second;
			envStrings.push_back(envVar);

			//std::cout << "✅ Added env var: " << envVar << std::endl;  // Debug output
		}

		// Convert to char* array for execve
		std::vector<char*> envp;
		for (size_t i = 0; i < envStrings.size(); ++i) {
			envp.push_back(const_cast<char*>(envStrings[i].c_str()));
		}
		envp.push_back(NULL);

		// Prepare command arguments
		char* args[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(scriptPath.c_str()),
			NULL
		};

		execve(interpreter.c_str(), args, &envp[0]);

		// If we reach here, execve failed
		std::cerr << "❌ execve failed"<< std::endl;
		exit(1);
	} else {
		// Parent process: read the output
//		std::cout << "👨‍👩‍👧‍👦 Parent process: reading script output" << std::endl;
		// Close unused pipe ends
		close(inputPipe[0]);
		close(outputPipe[1]);

		// Send POST data to script if any
		std::string body = req.getBody();
		if (!body.empty() && req.getMethod() == "POST") {
//			std::cout << "📤 Sending POST data to script (" << body.size() << " bytes)" << std::endl;
			write(inputPipe[1], body.c_str(), body.size());
		}
		close(inputPipe[1]); // Close input pipe

		int status = 0;
		time_t start = time(NULL);
		bool timetokill = false;
		while (true) {
			pid_t result = waitpid(pid, &status, WNOHANG);

			if (result == pid) break; //child has finished
			if (result == -1) {
				std::cerr << "❌ waitpid error\n";
				break;
			}
			if (time(NULL) - start > 5) { //for a process kill if exceed timout of 5 seconds
				std::cerr << "⏰ CGI timeout, killing child 🔪🩸😵\n";
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				timetokill = true;
				break;
			}
			usleep(100000); //sleep for 100ms before retrying again.
		}
		if (timetokill) {
			std::cout << "⚠️ CGI script execution timed out" << std::endl;
			close(outputPipe[0]);
			return "HTTP/1.1 504 Gateway Timeout\r\n"
					"Content-Type: text/html\r\n\r\n"
					"<html><body><h1>504 Gateway Timeout</h1></body></html>";
		}
		// Read all output from the script
		std::string output;
		char buffer[8192];
		ssize_t bytesRead;
		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
			output.append(buffer, bytesRead);
		}

		close(outputPipe[0]);

		// Wait for child process to finish

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			std::cout << "✅ Script executed successfully" << std::endl;
		}
		else {
			std::cout << "⚠️ Script exited with status: " << WEXITSTATUS(status) << std::endl;
		}

		// Format the output as a proper HTTP response
		return formatCGIResponse(output);
	}
}

// Helper function to format CGI output as HTTP response
std::string formatCGIResponse(const std::string& scriptOutput) {
	if (scriptOutput.empty()) {
		std::cout << "⚠️ Script output is empty" << std::endl;
		return "";
	}

//	std::cout << "📋 Formatting CGI response (" << scriptOutput.size() << " bytes)" << std::endl;

	// Check if the script already included HTTP headers
	size_t headerEnd = scriptOutput.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		// Check if script provided its own status
		size_t statusPos = scriptOutput.find("Status:");
		if (statusPos != std::string::npos && statusPos < headerEnd) {
			// Extract status code from "Status: 418 I'm a teapot"
			size_t statusStart = statusPos + 7; // "Status:" length
			size_t statusEnd = scriptOutput.find("\r\n", statusStart);
			if (statusEnd != std::string::npos) {
				std::string statusLine = scriptOutput.substr(statusStart, statusEnd - statusStart);
				// Remove the Status: line from output and add proper HTTP status
				std::string cleanedOutput = scriptOutput;
				cleanedOutput.erase(statusPos, statusEnd - statusPos + 2); // +2 for \r\n

				return "HTTP/1.1 " + statusLine + "\r\n" + cleanedOutput;
			}
		}

		// Script provided headers but no status - default to 200
		if (scriptOutput.find("Content-Type:") < headerEnd) {
			return "HTTP/1.1 200 OK\r\n" + scriptOutput;
		}
	}

	// Script didn't provide headers, add them
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << scriptOutput.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << scriptOutput;
	return response.str();
}