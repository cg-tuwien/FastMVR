#pragma once

#include "RP311.h"

class RP311a : public RP311
{
protected:

	static const char* gs_GL;
	static const char* gs_GL_debug;

	virtual void buildProgram() override;

public:

	RP311a(
		bool debug,
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices);
};