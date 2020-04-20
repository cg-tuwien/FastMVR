#include "RP701.h"
#include "RenderResources.h"
#include <array>
#include <stb_image_write.h>

// RP701
// Can only render the entire scene at once.
// Can only render one view.
// Assumes that model matrices are baked into the geometry.
// Uses separate pass and framebuffer with separate m_textures attached for each depth range.
// Uses a simple uniform to set the depth range in each pass.

#pragma region ShaderCode

const char* RP701::vs_GL = R"(
#version 450

in layout(location = 0) vec3 position;

uniform mat4 V;

void main()
{
	gl_Position = V * vec4(position, 1.0f);
}
)";

const char* RP701::gs_GL = R"(
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

const char* RP701::fs_GL = R"(
#version 450
out layout(location = 0) uint d_color;

void main()
{
	d_color = gl_PrimitiveID;
}
)";

#pragma endregion

RP701::RP701(
	uint32_t width,
	uint32_t height,
	glm::mat4 view_matrix,
	std::vector<glm::vec2> depth_ranges, GeometryProviderBase& geometry)
	: DepthTechnique("later...", width, height, view_matrix, depth_ranges, geometry)
{
	m_name = "RP701";
}

std::array<std::string, 3> RP701::getShaders()
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

void RP701::doDepthRange(uint32_t p)
{
	glm::vec2 range = depth_ranges[p];
	m_resources->setUniform(RenderResources::UniformType::VEC2, "DEPTH_RANGE", &range);

	m_resources->activateFramebuffer(m_framebuffers[0][p]);
	m_resources->clear();
	m_resources->renderAll(geometry_provider(), {}); // TODO: pass in matrices in 2nd parameter
}

void RP701::init(RenderResources* r)
{
	m_resources = r;

	// First template parameter is 1: no division by depth, one depth layer for entire scene.
	m_textures = std::vector<std::vector<int>>(1, std::vector<int>(depth_ranges.size()));
	m_renderbuffers = std::vector<std::vector<int>>(1, std::vector<int>(depth_ranges.size()));
	m_framebuffers = std::vector<std::vector<int>>(1, std::vector<int>(depth_ranges.size()));

	RenderResources::RenderbufferConfig r_config(m_width, m_height);
	RenderResources::FramebufferConfig f_config;
	RenderResources::ArrayTextureConfig c_config;
	c_config.width = m_width;
	c_config.height = m_height;
	c_config.format = RenderResources::TextureFormat::R32UI;

	for (int i = 0; i < depth_ranges.size(); i++)
	{
		m_textures[0][i] = m_resources->buildTexture(c_config);
		m_renderbuffers[0][i] = m_resources->buildRenderbuffer(r_config);
		m_framebuffers[0][i] = m_resources->buildFramebuffer(f_config);

		m_resources->combine(m_framebuffers[0][i], { m_textures[0][i] }, m_renderbuffers[0][i]);
		m_resources->enableTargets(m_framebuffers[0][i], { 0 });

		if (!m_resources->checkFramebuffer(m_framebuffers[0][i]))
		{
			throw std::runtime_error("Problem making framebuffer!");
		}
	}
}

void RP701::begin()
{
	// Needs baked, has no concept of matrices, hence "true"
	m_resources->begin(m_program, true);
	m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "V", &view_matrix);
}

void RP701::dumpImages(const char* directory)
{
	uint32_t w = m_width;
	uint32_t h = m_height;
	std::vector<uint32_t> data(m_width * m_height);
	for (int v = 0; v < depth_ranges.size(); v++)
	{
		m_resources->fetchTextureLayer(m_textures[0][v], 0, data);
		for (int i = 0; i < data.size(); i++)
		{
			data[i] |= 0xFF000000; // set full alpha. hack! remove later!
		}
		std::string name = std::string(directory) + std::string("v") + std::to_string(0) + std::string("_d") + std::to_string(v) + std::string(".png");
		stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
	}
}