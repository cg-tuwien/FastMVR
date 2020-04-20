#include "RP701.h"
#include "RenderResources.h"
#include <array>

// RP001a
// Basic Multipass-Multipass strategy. Keeps one texture for each depth layer and each view point.
// Meaning that if you have V view points and L layers, you end up with V*L textures.
// Uses only a single color target per pass, hences V*L passes. Uses a single target per framebuffer.
// Hence V*L framebuffers. Switches targets by switching framebuffer.
// Eliminates invalid depth layers via geometry shader.

const char* RP001a::gs_GL = R"(
#version 450

uniform vec2 DEPTH_RANGE;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
	float z1 = gl_in[0].gl_Position.z;
	float z2 = gl_in[1].gl_Position.z;
	float z3 = gl_in[2].gl_Position.z;

	float min_z = min(z1, min(z2, z3));

	if(min_z >= DEPTH_RANGE.x && min_z < DEPTH_RANGE.y)
	{
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[0].gl_Position;
		EmitVertex();
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[1].gl_Position;
		EmitVertex();
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[2].gl_Position;
		EmitVertex();	
		EndPrimitive();
	}
}
)";

RP001a::RP001a(
	uint32_t width,
	uint32_t height,
	std::vector<glm::vec2> depth_ranges,
	std::vector<glm::mat4> view_positions)
	: RP001(width, height, depth_ranges, view_positions)
{

}

std::array<std::string, 3> RP001a::getShaders()
{

	if (resources->name() == "GL")
	{
		return { RP001::vs_GL, RP001a::gs_GL, RP001::fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}

void RP001a::proceedDepthPass(uint32_t p)
{
	glm::vec2 range = depth_ranges[p];
	resources->setUniform(RenderResources::UniformType::VEC2, "DEPTH_RANGE", &range);
}

