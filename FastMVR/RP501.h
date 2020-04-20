#pragma once

#include "RP202.h"
#include <cstdint>
#include <vector>
#include <glm/matrix.hpp>

class RP501 : public RP202
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP501(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void init(RenderResources* resources) override;
};
