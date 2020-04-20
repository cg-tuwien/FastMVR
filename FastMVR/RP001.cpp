#include "RP001.h"
#include <string>
#include "RenderResources.h"
#include <array>
#include <stb_image_write.h>

// RP001
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses separate pass and framebuffer with separate m_textures attached for each view point rendering.
// Uses a simple uniform to set the view matrix in each pass.

#pragma region ShaderCode

const char* RP001::vs_GL = R"(
#version 450

in layout(location = 0) vec3 position;

uniform mat4 V;

void main()
{
	gl_Position = V * vec4(position, 1.0f);
}
)";

const char* RP001::gs_GL = R"()";

const char* RP001::fs_GL = R"(
#version 450
out layout(location = 0) uint d_color;

void main()
{
	d_color = gl_PrimitiveID;
}
)";

#pragma endregion

RP001::RP001(
	uint32_t width, 
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry) 
	:
	ViewTechnique("later...", width, height, view_positions, geometry)
{
	m_name = "RP001";
}

std::array<std::string, 3> RP001::getShaders()
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

void RP001::init(RenderResources* r)
{
	m_resources = r;

	// First template parameter is 1: no division by depth, one depth layer for entire scene.
	m_textures = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));
	m_renderbuffers = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));
	m_framebuffers = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));

	RenderResources::RenderbufferConfig r_config(m_width, m_height);
	RenderResources::FramebufferConfig f_config;
	RenderResources::ArrayTextureConfig c_config;
	c_config.width = m_width;
	c_config.height = m_height;
	c_config.format = RenderResources::TextureFormat::R32UI;

	for (int i = 0; i < view_matrices.size(); i++)
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

void RP001::begin()
{
	// Needs baked, has no concept of matrices, hence "true"
	m_resources->begin(m_program, true);
}

void RP001::doViewPosition(uint32_t p)
{
	m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "V", &view_matrices[p]);
	m_resources->activateFramebuffer(m_framebuffers[0][p]);
	m_resources->clear();
	m_resources->renderAll(geometry_provider(), { view_matrices[p] }); // Needs baked, has no concept of matrices, hence "true"
}

void RP001::dumpImages(const char* directory)
{
	uint32_t w = m_width;
	uint32_t h = m_height;
	std::vector<uint32_t> data(m_width * m_height);
	for (int v = 0; v < view_matrices.size(); v++)
	{
		m_resources->fetchTextureLayer(m_textures[0][v], 0, data);
		for (int i = 0; i < data.size(); i++)
		{
			data[i] |= 0xFF000000; // set full alpha. hack! remove later!
		}
		std::string name = std::string(directory) + std::string("v") + std::to_string(v) + std::string("_d") + std::to_string(0) + std::string(".png");
		stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
	}
}