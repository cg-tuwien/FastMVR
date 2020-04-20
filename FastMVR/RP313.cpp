#include "RP313.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>

// RP313
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates by using multi-draw call INDIRECT.
// Geometry shader selects layer for each view point based on draw call.
// Uses a uniform buffer to set all view matrices at once.
// Speeds things up by using Pass-Through (NVIDIA only).

RP313::RP313(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP312a(width, height, view_positions, geometry)
{
	m_name = "RP313";
}

void RP313::begin()
{
	//RP312a::begin();

	//auto geometries = geometry_provider().getGeometryForMatrices(view_matrices);

	//std::vector<RenderResources::DrawElementsIndirectCommand> indirects;
	//for (auto& r : geometries) 
	//{
	//	for (int i = 0; i < view_matrices.size(); i++)
	//	{
	//		indirects.push_back(r.m_drawcall_data);
	//	}
	//}
	//assert(indirects.size() == geometries.size() * view_matrices.size());

	//int buff = resources()->buildIndirectsBuffer(RenderResources::BufferConfig::Usage::STATIC_DRAW, indirects);
	//resources()->setIndirect(buff);
}

void RP313::start()
{
	//m_resources->clear();
	//m_resources->renderIndirectMulti((int)view_matrices.size(), geometry_provider(), view_matrices);
}