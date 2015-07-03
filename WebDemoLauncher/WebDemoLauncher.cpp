// WebDemoLauncher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WebServer.h"
#include <iostream>

void launchBrowser(std::string url) {
	std::string command;
	command += "start " + url;
	system(command.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout <<
		"\n_________        ___.                 _________         __    \n" <<
		"\\_   ___ \\___.__.\\_ |__   ___________ \\_   ___ \\_____ _/  |_  \n" <<
		"/    \\  \\<   |  | | __ \\_/ __ \\_  __ \\/    \\  \\/\\__  \\\\   __\\ \n" <<
		"\\     \\___\\___  | | \\_\\ \\  ___/|  | \\/\\     \\____/ __ \\|  |   \n" <<
		" \\______  / ____| |___  /\\___  >__|    \\______  (____  /__|   \n" <<
		"        \\/\\/          \\/     \\/               \\/     \\/       \n" <<
		"_________                        __  .__                      \n" <<
		"\\_   ___ \\_______   ____ _____ _/  |_|__| ____   ____   ______\n" <<
		"/    \\  \\/\\_  __ \\_/ __ \\\\__  \\\\   __\\  |/  _ \\ /    \\ /  ___/\n" <<
		"\\     \\____|  | \\/\\  ___/ / __ \\|  | |  (  <_> )   |  \\\\___ \\ \n" <<
		" \\______  /|__|    \\___  >____  /__| |__|\\____/|___|  /____  >\n" <<
		"        \\/             \\/     \\/                    \\/     \\/ \n\n" <<
		"WebDemo Launcher v0.1\n" <<
		"https://github.com/Suva/WebDemoLauncher\n\n";

	WebServer ws;
	int port = ws.getPort();
	launchBrowser("http://localhost:" + std::to_string(port));
	ws.handle();
}