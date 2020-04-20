#pragma once

#include "RenderPipeline.h"
#include <glm/matrix.hpp>


class ViewTechnique : public RenderPipeline
{

protected:
	std::vector<glm::mat4> view_matrices;

	virtual void doViewPosition(uint32_t view) {};

	virtual void start() {};

public:
	ViewTechnique(std::string name, uint32_t width, uint32_t height, std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void produce() override;

	enum class MultiViewDuplication
	{
		MULTIPASS,
		MULTIDRAW_INDIRECT,
		MULTIDRAW,
		GEOMETRY_SHADER_LOOP,
		GEOMETRY_SHADER_INSTANCING,
		INSTANCING,
		GIANT_INDEX_BUFFER,
		OVR_MULTIVIEW
	};
	enum class MultiViewMedium
	{
		SEPARATE_TEXTURES,
		TEXTURE_ARRAY,
		GIANT_TEXTURE,
		TEXTURE_ARRAY_OF_GIANT_TEXTURES,
		MULTIPLE_GIANT_TEXTURES,
		MULTIPLE_TEXTURE_ARRAYS
	};
	enum class MultiViewChange
	{
		NONE,
		FRAMEBUFFER,
		LAYER,
		VIEWPORT,
		CLIP_PLANES,
		FRAMEBUFFER_THEN_NONE,
		FRAMEBUFFER_THEN_VIEWPORT,
		LAYER_THEN_NONE,
		LAYER_THEN_VIEWPORT,
		FRAMEBUFFER_THEN_LAYER
	};
	enum class MultiViewPassThrough
	{
		DISABLED,
		ENABLED
	};

	auto get_view_matrix(size_t i = 0) const { return view_matrices[i]; }
};