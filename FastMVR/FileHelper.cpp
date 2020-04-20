#include "FileHelper.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void FileHelper::readFullFile(const char* filename, std::string& content)
{
	std::ifstream f(filename, std::ios_base::in);
	auto filesize = std::filesystem::file_size(filename);
	content = std::string(filesize, ' ');
	f.read(content.data(), filesize);
}

void FileHelper::readFullFileBinary(const char* filename, std::vector<std::uint8_t>& bytes)
{
	std::ifstream f(filename, std::ios_base::in | std::ios_base::binary);
	auto filesize = std::filesystem::file_size(filename);
	bytes.resize(filesize);
	f.read((char*)bytes.data(), filesize);
}