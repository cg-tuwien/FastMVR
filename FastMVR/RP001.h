#pragma once

#include "ViewTechniquePipeline.h"
#include <cstdint>
#include <vector>
#include <glm/matrix.hpp>

class RP001 : public ViewTechnique
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;
	static const char* fs_GL;

	virtual void doViewPosition(uint32_t view) override;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP001(
		uint32_t width, 
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void init(RenderResources* resources) override;

	virtual void begin() override;

	virtual void dumpImages(const char* directory) override;
};
