// WebDemoLauncher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WebServer.h"


int _tmain(int argc, _TCHAR* argv[])
{
	WebServer ws;
	ws.handle();
	Sleep(10000);
}

