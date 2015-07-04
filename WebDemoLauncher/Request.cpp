#include "stdafx.h"
#include "Request.h"


Request::Request(std::string request)
{
	extractFileName(request);
	detectFileType();
}


void Request::detectFileType()
{
	fileType = "application/octet-stream";

	int dotPosition = fileName.find(".");

	if (dotPosition >= 0) {
		std::string extension = fileName.substr(dotPosition + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

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
	}
}

void Request::extractFileName(std::string request)
{
	int firstSpace = request.find(" ") + 1;
	int secondSpace = request.find(" ", firstSpace);

	fileName = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);

	std::replace(fileName.begin(), fileName.end(), '/', '\\');

	if (fileName.length() == 0) {
		fileName = "index.html";
	}
}

Request::~Request()
{
}
