#pragma once
#include <string>

class Request
{
public:
	std::string fileName;
	std::string fileType;

	Request(std::string);
	~Request();
};

