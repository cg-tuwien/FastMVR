#include "RP401.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>
#include <regex>
#include "RP312.h"

// RP401
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses one pass for all view points, renders to a giant texture.
// Duplicates by using multi-draw call.
// Geometry shader selects viewport for each view point based on draw call.
// Uses a uniform buffer to set all view matrices at once.
// Speeds things up by using Pass-Through (NVIDIA only).

// NOTE: This is where it gets BAD. 401 inherits from 402, but has to overwrite the 
// doView method to make sure it is empty... and it duplicates shader code that was used
// in RP312... a better design for inherited/modular
// behavior would be cool!

#pragma region ShaderCode

const char* RP401::vs_GL = R"(
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

const char* RP401::gs_GL = R"(
#version 450

#extension GL_NV_geometry_shader_passthrough : require

layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

layout(location = 0) in int g_draw[];

void main()
{
	gl_ViewportIndex = g_draw[0];
	gl_PrimitiveID = gl_PrimitiveIDIn;
}
)";

#pragma endregion

RP401::RP401(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP402(width, height, view_positions, geometry)
{
	m_name = "RP401";
}

std::array<std::string, 3> RP401::getShaders()
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

void RP401::begin()
{
	RP402::begin(); //Make that giant texture

	std::vector<RenderResources::ViewportConfig> viewports(view_matrices.size());
	for (int i = 0; i < view_matrices.size(); i++)
	{
		viewports[i].x = i * (float)m_width;
		viewports[i].y = 0;
		viewports[i].width = (float)m_width;
		viewports[i].height = (float)m_height;
	}
	m_resources->activateViewports(viewports);

	// Update uniform buffer with view matrix data.
	RenderResources::BufferConfig conf(sizeof(glm::mat4) * (uint32_t)view_matrices.size(), RenderResources::BufferConfig::Usage::STREAM_DRAW);
	int buff = m_resources->buildBuffer(conf);
	m_resources->bufferData(buff, 0, view_matrices.data(), sizeof(glm::mat4) * (uint32_t)view_matrices.size());
	m_resources->bindUniformBufferRange(buff, "Vs", sizeof(glm::mat4) * (uint32_t)view_matrices.size());
}

void RP401::start()
{
	m_resources->clear();
	m_resources->renderAllMulti((int)view_matrices.size(), geometry_provider(), view_matrices);
}

void RP401::doViewPosition(uint32_t view)
{
}