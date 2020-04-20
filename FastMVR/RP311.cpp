#include "RP311.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <regex>
#include <array>

// RP311
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates by creating a via geometry shader instances.
// Geometry shader selects layer for each view point based on invocation.
// Uses a uniform buffer to set all view matrices at once.


#pragma region ShaderCode

const char* RP311::gs_GL = R"(
#version 450

layout(std140) uniform Vs
{
	mat4 V[32];
};

layout(triangles, invocations = %NUM_INVOCATIONS%) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
    const int inv = gl_InvocationID;
	const int id = gl_PrimitiveIDIn;
    const mat4 view = V[gl_InvocationID];

	for(int i = 0; i < 3; i++)
	{
		gl_Layer = inv;
		gl_PrimitiveID = id;
		gl_Position = view * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)";

#pragma endregion

RP311::RP311(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP202( width, height, view_positions, geometry)
{
	m_name = "RP311";
}

std::array<std::string, 3> RP311::getShaders()
{
	if (m_resources->name() == "GL")
	{
		// Patch number of geometry shader invocations with the number of view points.
		// Only works if we know the number of view points at shader compile time.

		std::string text = std::string(gs_GL);
		std::regex to_replace("%NUM_INVOCATIONS%");
		std::string patched_gs = std::regex_replace(text.c_str(), to_replace, std::to_string(view_matrices.size()));

		return { vs_GL, patched_gs.c_str(), fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}