#pragma once

#include "RenderPipeline.h"
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

class DepthTechnique : public RenderPipeline
{
protected:
	std::vector<glm::vec2> depth_ranges;

	glm::mat4 view_matrix;

	virtual void doDepthRange(uint32_t view) {};

	virtual void start() {};

public:

	enum class DepthLayerSorting { MULTIPASS, GEOMETRY_SHADER };
	enum class DepthLayerMedium { SEPARATE_TEXTURES, TEXTURE_ARRAY, GIANT_TEXTURE };
	enum class DepthLayerChange { FRAMEBUFFER, VIEWPORT, VIEWPORTARRAY, CLIPPED };

	DepthTechnique(std::string name, uint32_t width, uint32_t height, glm::mat4 view_matrix, std::vector<glm::vec2> depth_ranges, GeometryProviderBase& geometry);

	virtual void produce() override;

};