#include "RP312.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>


// RP312
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to an array texture.
// Duplicates by using multi-draw call.
// Geometry shader selects layer for each view point based on draw call.
// Uses a uniform buffer to set all view matrices at once.

#pragma region ShaderCode

const char* RP312::vs_GL = R"(
#version 460

layout(std140) uniform Vs
{
	mat4 V[32];
};

in layout(location = 0) vec3 position;
out layout(location = 0) int g_draw;

void main()
{
	g_draw = gl_DrawID;
	gl_Position = V[gl_DrawID] * vec4(position, 1.0f);
}
)";

const char* RP312::gs_GL = R"(
#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in layout(location = 0) int g_draw[];

void main()
{
	for(int i = 0; i < 3; i++)
	{
		gl_Layer = g_draw[i];
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)";

#pragma endregion

RP312::RP312(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP202(width, height, view_positions, geometry)
{
	m_name = "RP312";
}

std::array<std::string, 3> RP312::getShaders()
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

void RP312::start()
{
	m_resources->clear();
	m_resources->renderAllMulti((int)view_matrices.size(), geometry_provider(), view_matrices);
}