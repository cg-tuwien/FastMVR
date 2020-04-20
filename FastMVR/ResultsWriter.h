#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <string>
#include <iostream>

/**	Writes some results to a csv-file
 */
class ResultsWriter
{
public:
	ResultsWriter() = default;
	ResultsWriter(const ResultsWriter&) = default;
	ResultsWriter(ResultsWriter&&) = default;
	ResultsWriter& operator=(const ResultsWriter&) = default;
	ResultsWriter& operator=(ResultsWriter&&) = default;
	~ResultsWriter() = default;
	ResultsWriter(std::string_view outputFolder, std::string_view techniqueName);
	bool HasBufferContents() const;
	void AddToBuffer(std::string_view contents, bool addNewline = false);
	void ResetBuffer();
	void WriteBufferToFile(std::string_view filename);
	void WriteBufferToAutomaticallyNamedFile();
	void CheckIfDirectoriesExistOrCreateThem();

	const auto& base_output_folder() const { return m_outputFolder; }
	const auto& technique_name() const { return m_techniqueName; }
	std::string full_output_directory_path() const;
	std::string get_output_path_for_filename(std::string_view filename) const;

private:
	std::string m_outputFolder;
	std::string m_techniqueName;
	std::string m_outputPath;
	
	std::string m_buffer;
};

