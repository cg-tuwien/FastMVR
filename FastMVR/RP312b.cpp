#include "RP313.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>

// RP312b
// Just to test one of the methods with using indirect. Nothing special.


RP313::RP313(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions)
	:
	RP312a(width, height, view_positions)
{
}

void RP313::begin()
{
	RP202::begin();

	std::vector<RenderResources::IndirectDrawing> indirects(view_matrices.size());
	for (int i = 0; i < view_matrices.size(); i++)
	{
		indirects[i].count = resources->totalIndices();
		indirects[i].instances = 1;
		indirects[i].offset = 0;
	}

	int buff = resources->buildIndirectsBuffer(RenderResources::BufferConfig::Usage::STATIC_DRAW, indirects);
	resources->setIndirect(buff);
}


void RP313::produce()
{
	resources->clear();
	resources->renderIndirectMulti(view_matrices.size());
}