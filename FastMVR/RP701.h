#pragma once
#include "DepthTechniquePipeline.h"

class RP701 : public DepthTechnique
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;
	static const char* fs_GL;

	virtual std::array<std::string, 3> getShaders() override;

	virtual void doDepthRange(uint32_t view) override;

public:

	RP701(
		uint32_t width,
		uint32_t height,
		glm::mat4 view_matrix,
		std::vector<glm::vec2> depth_ranges, GeometryProviderBase& geometry);

	void begin() override;

	void init(RenderResources* resources) override;

	virtual void dumpImages(const char* directory) override;
};