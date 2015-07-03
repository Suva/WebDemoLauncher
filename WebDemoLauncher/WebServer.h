#pragma once
#include <exception>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>

struct BinaryData {
	char *data;
	int length;
};

class WebServer
{
private:
	WSADATA wsaData;
	SOCKET ListenSocket;

	int initWinsock(void);
	int bindPort(void);
	bool isEndOfRequest(std::string request);
	void handleRequest(SOCKET ClientSocket, std::string request);
	long getFileSize(std::string filename);
	void transferFile(SOCKET ClientSocket, std::string fileName);	
	
public:
	WebServer();
	int handle();

	~WebServer();
};

class NetworkException : public std::exception {
private: 
	char *reason;
public:
	NetworkException(char *message) {
		reason = message;
	}
};
