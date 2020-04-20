#include "RP302.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <regex>
#include <array>

// RP302
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates by creating a giant index buffer that consecutively references each triangle as often as there are views.
// Geometry shader selects layer for each view point based on primitive ID.
// Uses a uniform buffer to set all view matrices at once.

#pragma region ShaderCode

const char* RP302::gs_GL = R"(
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform Vs
{
	mat4 V[32];
};

void main()
{
	const int num_views = %NUM_VIEWS%;
	const int layer = gl_PrimitiveIDIn % num_views;
	const int id = gl_PrimitiveIDIn / num_views;

	for(int i = 0; i < 3; i++)
	{
		gl_Layer = layer;
		gl_PrimitiveID = id;
		gl_Position = V[layer] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)";

#pragma endregion

RP302::RP302(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP202(width, height, view_positions, geometry)
{
	m_name = "RP202";
}

std::array<std::string, 3>RP302::getShaders()
{
	if (m_resources->name() == "GL")
	{
		std::string text = std::string(gs_GL);
		std::regex to_replace("%NUM_VIEWS%");
		std::string patched_gs = std::regex_replace(text.c_str(), to_replace, std::to_string(view_matrices.size()));

		return { vs_GL, patched_gs, fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}

void RP302::begin()
{
	RP202::begin();
	// Creates a blown-up index buffer which simply multiplies the amount of geometry that is submitted
	m_resources->begin(m_program, (int)view_matrices.size());
}