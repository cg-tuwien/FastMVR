#pragma once

#include "RP312.h"

class RP312a : public RP312
{
protected:

	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP312a(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);
};