#pragma once
#include <string>

class Request
{
private:
	void detectFileType();
	void extractFileName(std::string request);

public:
	std::string fileName;
	std::string fileType;
	Request(std::string);
	~Request();
};

