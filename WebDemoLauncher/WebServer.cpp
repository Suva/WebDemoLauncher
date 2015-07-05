#include "stdafx.h"
#include "WebServer.h"

#pragma comment(lib, "Ws2_32.lib")

int random_number(int min_num, int max_num)
{
	int result = 0, low_num = 0, hi_num = 0;
	if (min_num<max_num)
	{
		low_num = min_num;
		hi_num = max_num + 1; // this is done to include max_num in output.
	}
	else {
		low_num = max_num + 1;// this is done to include max_num in output.
		hi_num = min_num;
	}

	result = (rand() % (hi_num - low_num)) + low_num;
	return result;
}

WebServer::WebServer()
{
	srand((unsigned int)time(NULL));

	ListenSocket = INVALID_SOCKET;
	if (initWinsock() != ERROR_SUCCESS) {
		throw new NetworkException("Error initializing winsock");
	}

	int numRetries = 10;
	int bindResult;
	do { 
		port = random_number(50000, 60000);
		bindResult = bindPort(port);
	} while (bindResult != ERROR_SUCCESS && numRetries++ > 0);

	if (bindResult != ERROR_SUCCESS) {
		WSACleanup();
		throw new NetworkException("Binding failed");
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

int WebServer::bindPort(int port) {
	int iResult;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	char portString[16];
	sprintf(portString, "%i", port);

	std::cout << "Trying to bind to port " << portString << "...";

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, portString, &hints, &result);
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
		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "done" << std::endl;

	return 0;
}

#define BUFFER_SIZE 1024

bool WebServer::ReadData(Connection& conn)
{
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);

	int nBytes = recv(conn.sd, buf, BUFFER_SIZE - 1, 0);
	if (nBytes == 0) {
		std::cout << "Socket " << conn.sd <<
			" was closed by the client. Shutting down." << std::endl;
		return false;
	}
	else if (nBytes == SOCKET_ERROR) {
		// Something bad happened on the socket.  It could just be a
		// "would block" notification, or it could be something more
		// serious.  Let caller handle the latter case.  WSAEWOULDBLOCK
		// can happen after select() says a socket is readable under
		// Win9x: it doesn't happen on WinNT/2000 or on Unix.
		int err;
		int errlen = sizeof(err);
		getsockopt(conn.sd, SOL_SOCKET, SO_ERROR, (char*)&err, &errlen);
		return (err == WSAEWOULDBLOCK);
	}

	conn.readbuf.append(buf);

	if (isEndOfRequest(conn.readbuf)) {
		Request request = Request(conn.readbuf);
		conn.readbuf.clear();
		handleRequest(conn, request);	
	}

	return true;
}

bool WebServer::WriteData(Connection& conn)
{
	if (conn.writebuf.length() == 0) {
		return true; // nothing to write
	}

	int nBytes = send(conn.sd, conn.writebuf.data(), conn.writebuf.length(), 0);
	if (nBytes == SOCKET_ERROR) {
		// Something bad happened on the socket.  Deal with it.
		int err;
		int errlen = sizeof(err);
		getsockopt(conn.sd, SOL_SOCKET, SO_ERROR, (char*)&err, &errlen);
		return (err == WSAEWOULDBLOCK);
	}

	if (nBytes == conn.writebuf.length()) {
		// Everything got sent, so take a shortcut on clearing buffer.
		conn.writebuf.clear();
		if (conn.shutdownTriggered)
		{
			shutdownTriggered = true;
			std::cout << "Shutdown triggered by remote, closing program." << std::endl;
			return 0;
		}
	}
	else {
		// We sent part of the buffer's data.  Remove that data from
		// the buffer.
		conn.writebuf = conn.writebuf.substr(nBytes);
	}

	return true;
}

void WebServer::SetupFDSets(fd_set& ReadFDs, fd_set& WriteFDs, fd_set& ExceptFDs)
{
	FD_ZERO(&ReadFDs);
	FD_ZERO(&WriteFDs);
	FD_ZERO(&ExceptFDs);

	// Add the listener socket to the read and except FD sets, if there
	// is one.
	if (ListenSocket != INVALID_SOCKET) {
		FD_SET(ListenSocket, &ReadFDs);
		FD_SET(ListenSocket, &ExceptFDs);
	}

	// Add client connections
	ConnectionList::iterator it = connections.begin();
	while (it != connections.end()) {
		FD_SET(it->sd, &ReadFDs);

		if (it->writebuf.length() > 0) {
			// There's data still to be sent on this socket, so we need
			// to be signaled when it becomes writable.
			FD_SET(it->sd, &WriteFDs);
		}

		FD_SET(it->sd, &ExceptFDs);

		++it;
	}
}

int WebServer::handle() {
	sockaddr_in sinRemote;
	int nAddrSize = sizeof(sinRemote);
	
	timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	while(1){
		fd_set ReadFDs, WriteFDs, ExceptFDs;
		SetupFDSets(ReadFDs, WriteFDs, ExceptFDs);

		if (select(0, &ReadFDs, &WriteFDs, &ExceptFDs, &timeout) > 0) {
			if (FD_ISSET(ListenSocket, &ReadFDs)) {
				SOCKET sd = accept(ListenSocket,
					(sockaddr*)&sinRemote, &nAddrSize);
				if (sd != INVALID_SOCKET) {
					// Tell user we accepted the socket, and add it to
					// our connection list.
					std::cout << "Accepted connection from " <<
						inet_ntoa(sinRemote.sin_addr) << ":" <<
						ntohs(sinRemote.sin_port) <<
						", socket " << sd << "." << std::endl;
					connections.push_back(Connection(sd));

					if ((connections.size() + 1) > 64) {
						// For the background on this check, see
						// www.tangentsoft.net/wskfaq/advanced.html#64sockets
						// The +1 is to account for the listener socket.
						std::cout << "WARNING: More than 63 client "
							"connections accepted.  This will not "
							"work reliably on some Winsock "
							"stacks!" << std::endl;
					}

					// Mark the socket as non-blocking, for safety.
					u_long nNoBlock = 1;
					ioctlsocket(sd, FIONBIO, &nNoBlock);
				}
				else {
					std::cerr << "accept() failed" << std::endl;
					return 1;
				}
			}

			ConnectionList::iterator it = connections.begin();
			while (it != connections.end()) {
				bool bOK = true;
				const char* pcErrorType = 0;

				// See if this socket's flag is set in any of the FD
				// sets.
				if (FD_ISSET(it->sd, &ExceptFDs)) {
					bOK = false;
					pcErrorType = "General socket error";
					FD_CLR(it->sd, &ExceptFDs);
				}
				else {
					if (FD_ISSET(it->sd, &ReadFDs)) {
						bOK = ReadData(*it);
						pcErrorType = "Read error";
						FD_CLR(it->sd, &ReadFDs);
					}
					if (FD_ISSET(it->sd, &WriteFDs)) {
						bOK = WriteData(*it);
						pcErrorType = "Write error";
						FD_CLR(it->sd, &WriteFDs);
					}
				}

				if (!bOK) {
					// Something bad happened on the socket, or the
					// client closed its half of the connection.  Shut
					// the conn down and remove it from the list.
					int err;
					int errlen = sizeof(err);
					getsockopt(it->sd, SOL_SOCKET, SO_ERROR,
						(char*)&err, &errlen);
					
					closesocket(it->sd);
					connections.erase(it);
					it = connections.begin();
				}
				else {
					// Go on to next connection
					++it;
				}
			}
		
		} 
		if (heartbeat && (time(0) - heartbeat >= 3)) {
			std::cout << "Heartbeat lost, closing program." << std::endl;
			return 0;
		}

		if (shutdownTriggered)
		{
			return 0;
		}
	}

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

void WebServer::createResponse(Connection &conn, std::string status, std::string contentType, std::string payload)
{
	conn.writebuf +=
		"HTTP/1.1" + status + "\r\n" +
		"Content-Type: " + contentType + "\r\n" +
		"Content-Length: " + std::to_string(payload.length()) + "\r\n\r\n" +
		payload;
}

void WebServer::handleRequest(Connection& conn, Request request) 
{
	// Special requests
	if (request.fileName == "shutdown") {
		conn.shutdownTriggered = true;
		createResponse(conn, "200 OK", "text/plain", "Shutdown initiated.");
		return;
	}

	if (request.fileName == "heartbeat") {
		if(!heartbeat)
			std::cout << "Heartbeat detected, will shut down server when lost." << std::endl;
		heartbeat = time(0);	
	}

	if (request.fileName == "wdl.js") {
		char *script =
			"window.onbeforeunload = function() { var r = new XMLHttpRequest(); r.open('GET', '/shutdown', false); r.send(); }";
		createResponse(conn, "200 OK", "application/javascript", script);
	}

	// Normal file handling
	std::string fileContents = getFile(request.fileName);
	if (fileContents.length() == 0) {
		createResponse(conn, "404 Not found", "text/html", "<h1>File not found</h1>");
	} else {
		createResponse(conn, "200 OK", request.fileType, fileContents);
	}
}

int WebServer::getPort()
{
	return port;
}

std::string WebServer::getFile(std::string fileName)
{
	FILE* f = fopen(fileName.c_str(), "rb");

	if (!f) {
		return "";
	}

	char buf[BUFFER_SIZE];
	int totalCharactersRead = 0;
	std::string result;

	while (!feof(f)) {
		int charactersRead = fread(buf, sizeof(char), BUFFER_SIZE, f);
		result.append(buf, charactersRead);
		totalCharactersRead += charactersRead;
	}

	fclose(f);

	return result;
}

WebServer::~WebServer()
{
}
