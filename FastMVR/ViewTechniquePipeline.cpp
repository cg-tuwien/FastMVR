#include "ViewTechniquePipeline.h"
#include "RenderResources.h"


ViewTechnique::ViewTechnique(std::string name, uint32_t width, uint32_t height, std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry)
	:
	RenderPipeline(std::move(name), width, height, geometry),
	view_matrices(view_matrices)
{

}

void ViewTechnique::produce()
{
	start();
	for (int i = 0; i < view_matrices.size(); i++)
	{
		doViewPosition(i);
	}
}

