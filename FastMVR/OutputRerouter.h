#pragma once

#include <iostream>
#define NOMINMAX
#include <Windows.h>


class debugStreambuf : public std::streambuf {
public:
	virtual int_type overflow(int_type c = EOF) {
		if (c != EOF) {
			TCHAR buf[] = { (TCHAR)c, (TCHAR)'\0' };
			OutputDebugString(buf);
		}
		return c;
	}
};

class Cout2VisualStudioDebugOutput {

	debugStreambuf dbgstream;
	std::streambuf* default_stream;

public:
	Cout2VisualStudioDebugOutput() {
		default_stream = std::cout.rdbuf(&dbgstream);
	}

	~Cout2VisualStudioDebugOutput() {
		std::cout.rdbuf(default_stream);
	}
};