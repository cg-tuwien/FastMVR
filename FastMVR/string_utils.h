#pragma once
#include <string>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <sstream>
#include <glm/glm.hpp>

static std::string trim_spaces(std::string_view s)
{
	auto pos1 = s.find_first_not_of(' ');
	auto pos2 = s.find_last_not_of(' ');
	if (std::string_view::npos == pos1 || std::string_view::npos == pos2) {
		return std::string(s);
	}
	return std::string(s.substr(pos1, pos2-pos1+1));
}

#ifdef USE_BACKSPACES_FOR_PATHS
const char SEP_TO_USE = '\\';
const char SEP_NOT_TO_USE = '/';
#else
const char SEP_TO_USE = '/';
const char SEP_NOT_TO_USE = '\\';
#endif

static std::string clean_up_path(std::string_view path)
{
	auto cleaned_up = trim_spaces(path);
	int consecutive_sep_cnt = 0;
	for (int i = 0; i < cleaned_up.size(); ++i) {
		if (cleaned_up[i] == SEP_NOT_TO_USE) {
			cleaned_up[i] = SEP_TO_USE;
		}
		if (cleaned_up[i] == SEP_TO_USE) {
			consecutive_sep_cnt += 1;
		}
		else {
			consecutive_sep_cnt = 0;
		}
		if (consecutive_sep_cnt > 1) {
			cleaned_up = cleaned_up.substr(0, i) + (i < cleaned_up.size() - 1 ? cleaned_up.substr(i + 1) : "");
			consecutive_sep_cnt -= 1;
			--i;
		}
	}
	return cleaned_up;
}

static std::string extract_file_name(std::string_view path)
{
	auto cleaned_path = clean_up_path(path);
	auto last_sep_idx = cleaned_path.find_last_of(SEP_TO_USE);
	if (std::string::npos == last_sep_idx) {
		return cleaned_path;
	}
	return cleaned_path.substr(last_sep_idx + 1);
}

static std::string extract_base_path(std::string_view path)
{
	auto cleaned_path = clean_up_path(path);
	auto last_sep_idx = cleaned_path.find_last_of(SEP_TO_USE);
	if (std::string::npos == last_sep_idx) {
		return cleaned_path;
	}
	return cleaned_path.substr(0, last_sep_idx + 1);
}

static std::string extract_filename_without_extension(std::string_view path)
{
	auto filename = extract_file_name(path);
	auto dot_idx = filename.find_last_of('.');
	if (std::string::npos == dot_idx || dot_idx < 1) {
		return filename;
	}
	return filename.substr(0, dot_idx);
}

static std::string combine_paths(std::string_view first, std::string_view second)
{
	return clean_up_path(std::string(first) + SEP_TO_USE + std::string(second));
}


static std::string from_file(const std::string& path)
{
	std::ifstream stream(path, std::ifstream::in);
	if (!stream.good() || !stream || stream.fail())
	{
		throw std::runtime_error("Unable to load shader source from path: " + path);
	}
	std::string filecontents = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	if (filecontents.empty())
	{
		std::cout << "WARNING: Filecontents empty when loading shader from path " << path << std::endl;
	}
	return filecontents;
}

static std::string insert_into_shader_code_after_version(std::string orig, std::string addition)
{
	if (addition.empty()) {
		return orig;
	}

	std::string result;
	std::istringstream iss(orig);
	bool inserted = false;
	for (std::string line; std::getline(iss, line); )
	{
	    result += line + "\n";
		if (!inserted && line.find("#version") != std::string::npos) {
			result += addition + "\n";
			inserted = true;
		}
	}

	if (inserted) {
		return result;
	}
	else {
		return addition + "\n" + orig;
	}
}
