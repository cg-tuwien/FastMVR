#pragma once

#include "RP301.h"

class RP301a : public RP301
{
protected:

	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP301a(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);
};