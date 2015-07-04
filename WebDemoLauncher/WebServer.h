#pragma once
#include <exception>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "Request.h"

struct BinaryData {
	char *data;
	int length;
};

struct Connection {
	SOCKET sd;
	std::string readbuf;
	std::string writebuf;

	Connection(SOCKET sd_) : sd(sd_) { }
};

typedef std::vector<Connection> ConnectionList;

class WebServer
{
private:
	int port;
	WSADATA wsaData;
	SOCKET ListenSocket;
	ConnectionList connections;
	bool shutdownTriggered = false;

	int initWinsock(void);
	int bindPort(int);
	bool ReadData(Connection & conn);
	bool isEndOfRequest(std::string request);
	void createResponse(Connection &conn, std::string status, std::string contentType, std::string payload);
	void handleRequest(Connection&, Request);
	bool WriteData(Connection& conn);
	void SetupFDSets(fd_set& ReadFDs, fd_set& WriteFDs, fd_set& ExceptFDs);
	std::string getFile(std::string fileName);

public:
	WebServer();
	int getPort();

	int handle();

	~WebServer();
};

class NetworkException : public std::exception {
private: 
	std::string reason;
public:
	NetworkException(std::string message) {
		reason = message;
	}
};
