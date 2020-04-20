#pragma once

#include <string>
#include <vector>

class FileHelper
{
public:
	static void readFullFile(const char* filename, std::string& content);

	static void readFullFileBinary(const char* filename, std::vector<std::uint8_t>& bytes);
};
