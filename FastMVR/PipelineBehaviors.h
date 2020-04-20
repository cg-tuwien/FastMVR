#pragma once

#include "ViewTechniquePipeline.h"

class TextureArrayCreator : public ViewTechniquePipeline
{
public:
	virtual void init(RenderResources* r) override;

	TextureArrayCreator(uint32_t width, uint32_t height, std::vector<glm::mat4>& view_matrices)
		: ViewTechniquePipeline(width, height, view_matrices)
	{}
};

class OneFBStarterViewBufferCopier : public TextureArrayCreator
{
public:

	virtual void begin() override;
	
	OneFBStarterViewBufferCopier(uint32_t width, uint32_t height, std::vector<glm::mat4>& view_matrices)
		: TextureArrayCreator(width, height, view_matrices)
	{}
};