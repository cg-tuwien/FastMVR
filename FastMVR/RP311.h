#pragma once

#include "RP202.h"

class RP311 : public RP202
{
protected:

	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP311(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);
};