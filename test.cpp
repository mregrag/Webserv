#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <vector>

// Linux-specific headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class HTTPServer {
	private:
		int serverSocket;
		const int PORT;
		const int BUFFER_SIZE;
		bool running;

		std::string handleRequest(const std::string& request) {
			// Simple request parser
			if (request.find("GET") == 0) {
				std::string path;
				size_t pathStart = request.find(" ") + 1;
				size_t pathEnd = request.find(" ", pathStart);
				if (pathStart != std::string::npos && pathEnd != std::string::npos) {
					path = request.substr(pathStart, pathEnd - pathStart);
				}

				// Generate response based on path
				if (path == "/" || path == "/index.html") {
					return createResponse(200, "OK", "<html><body><h1>Welcome to C++98 HTTP Server</h1><p>This is a simple HTTP server implementation for Linux.</p></body></html>");
				} else if (path == "/about") {
					return createResponse(200, "OK", "<html><body><h1>About</h1><p>This is a basic HTTP server written in C++98 for Linux systems.</p></body></html>");
				} else {
					return createResponse(404, "Not Found", "<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body></html>");
				}
			}

			return createResponse(501, "Not Implemented", "<html><body><h1>501 Not Implemented</h1><p>Only GET requests are supported.</p></body></html>");
		}

		std::string createResponse(int statusCode, const std::string& statusText, const std::string& body) {
			std::string response = "HTTP/1.1 " + intToString(statusCode) + " " + statusText + "\r\n";
			response += "Content-Type: text/html\r\n";
			response += "Content-Length: " + intToString(body.length()) + "\r\n";
			response += "Connection: close\r\n";
			response += "\r\n";
			response += body;
			return response;
		}

		std::string intToString(int value) {
			char buffer[20];
			sprintf(buffer, "%d", value);
			return std::string(buffer);
		}

	public:
		HTTPServer(int port = 8080, int bufferSize = 4096) 
			: PORT(port), BUFFER_SIZE(bufferSize), running(false) {
			}

		~HTTPServer() {
			if (running) {
				stop();
			}
		}

		bool start() {
			serverSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (serverSocket < 0) {
				std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
				return false;
			}

			// Set socket option to reuse the address
			int opt = 1;
			if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
				std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
				close(serverSocket);
				return false;
			}

			struct sockaddr_in serverAddr;
			memset(&serverAddr, 0, sizeof(serverAddr));
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			serverAddr.sin_port = htons(PORT);

			if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
				std::cerr << "Bind failed: " << strerror(errno) << std::endl;
				close(serverSocket);
				return false;
			}

			if (listen(serverSocket, 10) < 0) {
				std::cerr << "Listen failed: " << strerror(errno) << std::endl;
				close(serverSocket);
				return false;
			}

			std::cout << "HTTP Server started on port " << PORT << std::endl;
			running = true;
			return true;
		}

		void run() {
			if (!running && !start()) {
				return;
			}

			while (running) {
				struct sockaddr_in clientAddr;
				socklen_t clientAddrSize = sizeof(clientAddr);
				int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

				if (clientSocket < 0) {
					std::cerr << "Accept failed: " << strerror(errno) << std::endl;
					continue;
				}

				// Convert client IP to string for logging
				char clientIP[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
				std::cout << "New connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

				char* buffer = new char[BUFFER_SIZE];
				memset(buffer, 0, BUFFER_SIZE);

				int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
				if (bytesReceived > 0) {
					std::string request(buffer, bytesReceived);
					std::string response = handleRequest(request);

					send(clientSocket, response.c_str(), response.length(), 0);
					std::cout << "Response sent to " << clientIP << std::endl;
				}

				delete[] buffer;
				close(clientSocket);
			}
		}

		void stop() {
			running = false;
			close(serverSocket);
			std::cout << "Server stopped." << std::endl;
		}
};

int main() {
	try {
		HTTPServer server(8080);
		std::cout << "Starting HTTP server on port 8080..." << std::endl;
		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
