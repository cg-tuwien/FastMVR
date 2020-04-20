#include "ResultsWriter.h"
#include "string_utils.h"
#include <fmt/format.h>
#define NOMINMAX
#include <Windows.h>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <filesystem>

ResultsWriter::ResultsWriter(std::string_view outputFolder, std::string_view techniqueName)
	: m_outputFolder(clean_up_path(outputFolder))
	, m_techniqueName(clean_up_path(techniqueName))
{
	CheckIfDirectoriesExistOrCreateThem();
}

void ResultsWriter::CheckIfDirectoriesExistOrCreateThem()
{
	std::filesystem::create_directories(m_outputFolder);
	// Check if the basis output folder exists:
	if (!CreateDirectoryA(m_outputFolder.c_str(), NULL) && ERROR_ALREADY_EXISTS != GetLastError())
	{
		throw new std::runtime_error(fmt::format("The folder '{}' does not exist and it couldn't be created.", m_outputFolder));
	}
	// Check if the technique's sub-output-folder exists:
	if (!CreateDirectoryA(full_output_directory_path().c_str(), NULL) && ERROR_ALREADY_EXISTS != GetLastError())
	{
		throw new std::runtime_error(fmt::format("The folder '{}' does not exist and it couldn't be created.", full_output_directory_path()));
	}
}

std::string ResultsWriter::full_output_directory_path() const
{
	return combine_paths(m_outputFolder, m_techniqueName);
}

std::string ResultsWriter::get_output_path_for_filename(std::string_view filename) const
{
	return combine_paths(full_output_directory_path(), clean_up_path(filename));
}

bool ResultsWriter::HasBufferContents() const
{
	return !m_buffer.empty();
}

void ResultsWriter::AddToBuffer(std::string_view contents, bool addNewline)
{
	m_buffer += contents;
	if (addNewline) {
		m_buffer += "\n";
	}
}

void ResultsWriter::ResetBuffer()
{
	m_buffer = "";
}

void ResultsWriter::WriteBufferToFile(std::string_view filename)
{
    std::ofstream out(filename);
    out << m_buffer;
    out.close();
    // Reset timeBuffer:
	ResetBuffer();
}

void ResultsWriter::WriteBufferToAutomaticallyNamedFile()
{
	if (!HasBufferContents()) {
		return;
	}
	
	CheckIfDirectoriesExistOrCreateThem();

	static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, 0x04); // maroon on black
	std::cout << "+=========================\n";
	std::cout << "|\n";
	std::cout << "| Writing results to file:\n";

	time_t timer;
    char timeBuffer[80];
    struct tm tm_info;

    time(&timer);
    localtime_s(&tm_info, &timer);

	strftime(timeBuffer, 26, "%Y-%m-%d_%Hh%Mm%S", &tm_info);
	auto outputfilename = get_output_path_for_filename(std::string(timeBuffer) + ".csv");
	for (int i=1; std::filesystem::exists(outputfilename); ++i) {
		outputfilename = get_output_path_for_filename(std::string(timeBuffer) + "_" + std::to_string(i) + ".csv");
	}

	WriteBufferToFile(outputfilename);

	std::cout << "| " << outputfilename << "\n";
	std::cout << "|\n";
	std::cout << "+=========================\n";
	SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
}