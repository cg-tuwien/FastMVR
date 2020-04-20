#include "RP312a.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>

// RP312a
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates by using multi-draw call.
// Geometry shader selects layer for each view point based on draw call.
// Uses a uniform buffer to set all view matrices at once.
// Speeds things up by using Pass-Through (NVIDIA only).

#pragma region ShaderCode

const char* RP312a::gs_GL = R"(
#version 450

#extension GL_NV_geometry_shader_passthrough : require

layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

layout(location = 0) in int g_draw[];

void main()
{
	gl_Layer = g_draw[0];
	gl_PrimitiveID = gl_PrimitiveIDIn;
}
)";

#pragma endregion

RP312a::RP312a(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP312(width, height, view_positions, geometry)
{
	m_name = "RP312a";
}

std::array<std::string, 3> RP312a::getShaders()
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