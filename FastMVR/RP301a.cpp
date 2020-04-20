#include "RP301a.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>

// RP301
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates via instanced rendering.
// Geometry shader selects layer for each view point based on instance ID.
// Uses a uniform buffer to set all view matrices at once.
// Speeds things up by using Pass-Through (NVIDIA only).

#pragma region ShaderCode

const char* RP301a::gs_GL = R"(
#version 450

#extension GL_NV_geometry_shader_passthrough : require

layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

layout(location = 0) in int g_instance[];

void main()
{
	gl_Layer = g_instance[0];
	gl_PrimitiveID = gl_PrimitiveIDIn;
}
)";

#pragma endregion

RP301a::RP301a(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP301(width, height, view_positions, geometry)
{
	m_name = "RP301a";
}

std::array<std::string, 3> RP301a::getShaders()
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