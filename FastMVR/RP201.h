#pragma once

#include "RP001.h"
#include <cstdint>
#include <vector>
#include <glm/matrix.hpp>

class RP201 : public RP001
{

public:

	RP201(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void init(RenderResources* resources) override;
};