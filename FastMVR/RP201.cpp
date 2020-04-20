#include "RP201.h"
#include <string>
#include "RenderResources.h"
#include <array>
#include <stb_image_write.h>

// RP201
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses separate pass and framebuffer with separate array m_textures attached for each view point rendering.
// Uses a simple uniform to set the view matrix in each pass.

RP201::RP201(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP001(width, height, view_positions, geometry)
{
	m_name = "RP201";
}

void RP201::init(RenderResources* r)
{
	m_resources = r;

	m_textures = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));
	m_renderbuffers = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));
	m_framebuffers = std::vector<std::vector<int>>(1, std::vector<int>(view_matrices.size()));

	RenderResources::ArrayTextureConfig c_config;
	c_config.width = m_width;
	c_config.height = m_height;
	c_config.num_layers = 1;
	c_config.format = RenderResources::TextureFormat::R32UI;

	RenderResources::ArrayTextureConfig d_config(m_width, m_height, 1, RenderResources::TextureFormat::DEPTH);
	RenderResources::FramebufferConfig f_config;

	for (int i = 0; i < view_matrices.size(); i++)
	{
		m_textures[0][i] = m_resources->buildArrayTexture(c_config);
		m_renderbuffers[0][i] = m_resources->buildArrayTexture(d_config);
		m_framebuffers[0][i] = m_resources->buildFramebuffer(f_config);

		m_resources->combineLayered(m_framebuffers[0][i], {m_textures[0][i]}, m_renderbuffers[0][i]);
		m_resources->enableTargets(m_framebuffers[0][i], { 0 });

		if (!m_resources->checkFramebuffer(m_framebuffers[0][i]))
		{
			throw std::runtime_error("Problem making framebuffer!");
		}
	}
}