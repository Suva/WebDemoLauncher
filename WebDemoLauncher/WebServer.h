#pragma once
#include <exception>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <vector>

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
	WSADATA wsaData;
	SOCKET ListenSocket;

	ConnectionList connections;

	int initWinsock(void);
	int bindPort(void);
	bool ReadData(Connection & conn);
	bool isEndOfRequest(std::string request);
	void handleRequest(Connection&, std::string);
	long getFileSize(std::string filename);
	void transferFile(Connection& conn, std::string fileName);
	bool WriteData(Connection& conn);
	void SetupFDSets(fd_set& ReadFDs, fd_set& WriteFDs, fd_set& ExceptFDs);

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
