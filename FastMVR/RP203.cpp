#include "RP203.h"
#include "RenderResources.h"
#include <array>

// RP202
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for each view point, renders to an array texture.
// Geometry shader with pass-through selects layer for each view point rendering.
// Uses a uniform buffer to set all view matrices at once.

const char* RP203::vs_GL = R"(
#version 450

uniform int LAYER;

layout(std140) uniform Vs
{
	mat4 V[32];
};

in layout(location = 0) vec3 position;

void main()
{
	gl_Position = V[LAYER] * vec4(position, 1.0f);
}
)";

const char* RP203::gs_GL = R"(
#version 450

#extension GL_NV_geometry_shader_passthrough : require

uniform int LAYER;

layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

void main()
{
	gl_Layer = LAYER;
	gl_PrimitiveID = gl_PrimitiveIDIn;
}
)";

RP203::RP203(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	: RP202(width, height, view_positions, geometry)
{
	m_name = "RP203";
}

std::array<std::string, 3> RP203::getShaders()
{
	if (m_resources->name() == "GL")
	{
		return { vs_GL, gs_GL, fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}

void RP203::start()
{
	m_resources->clear();
}

void RP203::doViewPosition(uint32_t p)
{
	m_resources->setUniform(RenderResources::UniformType::INT, "LAYER", &p);
	m_resources->renderAll(geometry_provider(), view_matrices);
}

