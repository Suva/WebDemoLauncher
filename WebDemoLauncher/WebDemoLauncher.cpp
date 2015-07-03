// WebDemoLauncher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WebServer.h"

void launchBrowser(std::string url) {
	std::string command;
	command += "start " + url;
	system(command.c_str());
}


int _tmain(int argc, _TCHAR* argv[])
{
	launchBrowser("http://localhost:8888");
	WebServer ws;
	ws.handle();
}