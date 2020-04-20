#include "DepthTechniquePipeline.h"
#include "RenderResources.h"

DepthTechniquePipeline::DepthTechniquePipeline(uint32_t width, uint32_t height, std::vector<glm::mat4> view_matrices)
	:
	RenderPipeline(width, height),
	view_matrices(view_matrices)
{

}

void DepthTechniquePipeline::produce()
{
	start();
	for (int i = 0; i < view_matrices.size(); i++)
	{
		doViewPosition(i);
	}
}

