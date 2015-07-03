#include "stdafx.h"
#include "Request.h"
#include <algorithm>


Request::Request(std::string request)
{
	int firstSpace = request.find(" ") + 1;
	int secondSpace = request.find(" ", firstSpace);
	
	fileName = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);

	std::replace(fileName.begin(), fileName.end(), '/', '\\');

	if (fileName.length() == 0) {
		fileName = "index.html";
	}

	fileType = "application/octet-stream";

	int dotPosition = fileName.find(".");

	if (dotPosition >= 0) {
		std::string extension = fileName.substr(dotPosition + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

		printf("File extension is: %s\n", extension.c_str());

		if (extension.compare("txt") == 0) {
			fileType = "text/plain";
		}

		if (extension.compare("html") == 0 || extension.compare("htm") == 0) {
			fileType = "text/html";
		}

		if (extension.compare("js") == 0) {
			fileType = "application/javascript";
		}

		if (extension.compare("css") == 0) {
			fileType = "text/css";
		}

		if (extension.compare("jpg") == 0 || extension.compare("jpeg") == 0) {
			fileType = "image/jpeg";
		}

		if (extension.compare("png") == 0 || extension.compare("png") == 0) {
			fileType = "image/png";
		}

		printf("File type is %s\n", fileType.c_str());
	} 
}


Request::~Request()
{
}
