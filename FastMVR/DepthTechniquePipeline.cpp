#include "DepthTechniquePipeline.h"
#include "RenderResources.h"


DepthTechnique::DepthTechnique(std::string name, uint32_t width, uint32_t height, glm::mat4 view_matrix, std::vector<glm::vec2> depth_ranges, GeometryProviderBase& geometry)
	:
	RenderPipeline(std::move(name), width, height, geometry),
	view_matrix(view_matrix),
	depth_ranges(depth_ranges)
{

}

void DepthTechnique::produce()
{
	start();
	for (int i = 0; i < depth_ranges.size(); i++)
	{
		doDepthRange(i);
	}
}

