#include "RP402.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>
#include <regex>

// RP402
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses multi-pass to render all view points, renders to a giant texture.
// Viewport is set before rendering each view.
// Updates view matrix with each pass.

RP402::RP402(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP001(width, height, view_positions, geometry)
{
	m_name = "RP402";
}

void RP402::init(RenderResources* r)
{
	m_resources = r;

	m_textures = { std::vector<int>(1) };
	m_renderbuffers = { std::vector<int>(1) };
	m_framebuffers = { std::vector<int>(1) };

	RenderResources::ArrayTextureConfig c_config;
	c_config.width = m_width * (uint32_t)view_matrices.size();
	c_config.height = m_height;
	c_config.num_layers = 1;
	c_config.format = RenderResources::TextureFormat::R32UI;

	RenderResources::RenderbufferConfig d_config(m_width * (uint32_t)view_matrices.size(), m_height);
	RenderResources::FramebufferConfig f_config;

	m_textures[0][0] = m_resources->buildTexture(c_config);
	m_renderbuffers[0][0] = m_resources->buildRenderbuffer(d_config);
	m_framebuffers[0][0] = m_resources->buildFramebuffer(f_config);

	m_resources->combine(m_framebuffers[0][0], { m_textures[0][0] }, m_renderbuffers[0][0]);
	m_resources->enableTargets(m_framebuffers[0][0], { 0 });

	if (!m_resources->checkFramebuffer(m_framebuffers[0][0]))
	{
		throw std::runtime_error("Problem making framebuffer!");
	}
}

std::array<std::string, 3> RP402::getShaders()
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

void RP402::begin()
{
	// Needs baked, has no concept of matrices, hence "true"
	m_resources->begin(m_program, true);
	m_resources->activateFramebuffer(m_framebuffers[0][0]);
}

void RP402::start()
{
	m_resources->clear();
}

void RP402::doViewPosition(uint32_t p)
{
	m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "V", &view_matrices[p]);

	RenderResources::ViewportConfig conf;
	conf.x = p * (float)m_width;
	conf.y = 0;
	conf.width = (float)m_width;
	conf.height = (float)m_height;

	m_resources->activateViewports({ conf });
	m_resources->renderAll(geometry_provider(), { view_matrices[p] }); // Needs baked, has no concept of matrices, hence "true"
}

void RP402::dumpImages(const char* directory)
{
	uint32_t w = m_width;
	uint32_t h = m_height;
	std::vector<uint32_t> data((w * view_matrices.size()) * h);

	for (int l = 0; l < view_matrices.size(); l++)
	{
		m_resources->fetchTextureLayer(m_textures[0][0], 0, l * w, 0, w, h, data);
		for (int i = 0; i < data.size(); i++)
		{
			data[i] |= 0xFF000000; // set full alpha. hack! remove later!
		}
		std::string name = std::string(directory) + std::string("v") + std::to_string(l) + std::string("_d") + std::to_string(0) + std::string(".png");
		stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
	}
}
