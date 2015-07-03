#include "stdafx.h"
#include "WebServer.h"
#include "Request.h"
#include <fstream>
#include <streambuf>
#include <direct.h>
#include <stdlib.h>


#pragma comment(lib, "Ws2_32.lib")

#define PORT "8888"

WebServer::WebServer()
{
	ListenSocket = INVALID_SOCKET;
	if (initWinsock() != ERROR_SUCCESS) {
		throw new NetworkException("Error initializing winsock");
	}

	if (bindPort() != ERROR_SUCCESS) {
		throw new NetworkException("Error binding port");
	}
}

int WebServer::initWinsock(void) {
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	return 0;
}

int WebServer::bindPort(void) {
	int iResult;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

int WebServer::handle() {
	SOCKET ClientSocket;

	ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	#define DEFAULT_BUFLEN 512

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN - 1;

	

	do {
		memset(recvbuf, 0, DEFAULT_BUFLEN);
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		std::string request;
		if (iResult > 0) {
			request += recvbuf;

			if (isEndOfRequest(request)) {
				handleRequest(ClientSocket, request);
			}

		}
		else if (iResult == 0){
			printf("Connection closing...\n");
		} else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	return 0;
}

bool WebServer::isEndOfRequest(std::string request) {
	for (unsigned int i = 0; i < request.length(); i++) {
		if (request.substr(i, 4).compare("\r\n\r\n") == COMP_EQUAL) {
			return true;
		}
	}
	return false;
}



void WebServer::handleRequest(SOCKET ClientSocket, std::string requestString) {
	Request request = Request(requestString);

	BinaryData data = getFileContents(request.fileName);

	printf("Requested file: %s\n", request.fileName.c_str());
	
	std::string response = "";

	if (data.length == 0) {
		response += "HTTP/1.1 404 Not found\r\nContent-Length: 0\r\n\r\n";
	} else {
		response += "HTTP/1.1 200 OK\r\nContent-Type:";
		response += request.fileType;
		response += "\r\nContent-Length: ";
		response += std::to_string(data.length);
		response += "\r\n\r\n";
	}

	int iSendResult = send(ClientSocket, response.c_str(), response.length(), 0);

	if (data.length > 0) {
		iSendResult = send(ClientSocket, data.data, data.length, 0);
	}
	
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		throw new NetworkException("Error sending response to client!");
	}
}

BinaryData WebServer::getFileContents(std::string fileName) {
	BinaryData res;
	res.length = 0;

	FILE* f = fopen(fileName.c_str(), "r");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t size = ftell(f);

		char* where = new char[size];

		rewind(f);
		fread(where, sizeof(char), size, f);

		fclose(f);

		res.data = where;
		res.length = size;
	}
	
	return res;
}

WebServer::~WebServer()
{
}
