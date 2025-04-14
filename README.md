# Webserv

This project is a custom HTTP server implemented in C++98. It is designed to handle multiple simultaneous connections using non-blocking I/O and `poll()`, mimicking behavior similar to lightweight servers like NGINX.

## 🚀 Features

- Non-blocking server with `poll()`
- HTTP/1.1 support: GET, POST, DELETE
- Configurable via custom configuration files
- Static file serving (HTML, CSS, JS, images)
- File upload support
- CGI execution (e.g., PHP, Python scripts)
- Custom error pages
- Compatible with modern web browsers

