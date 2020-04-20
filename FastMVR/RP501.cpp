#include "RP501.h"
#include <string>
#include "RenderResources.h"
#include <stb_image_write.h>
#include <array>
#include <regex>

// RP402
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses NVIDIA MVR to render all view points (4) at once, renders to array texture.
// Uses uniform buffer to update view matrices all at once.

#pragma region ShaderCode

const char* RP501::vs_GL = R"(
#version 450

#extension GL_OVR_multiview : require

layout (num_views = %NUM_VIEWS%) in;
layout(location = 0) in vec3 position;
layout(std140) uniform Vs
{
	mat4 V[32];
};

void main()
{
	gl_Position = V[gl_ViewID_OVR] * vec4(position, 1.0f);
}
)";

const char* RP501::gs_GL = R"(
)";


#pragma endregion

RP501::RP501(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_positions, GeometryProviderBase& geometry)
	:
	RP202(width, height, view_positions, geometry)
{
	m_name = "RP501";
}

std::array<std::string, 3> RP501::getShaders()
{
	if (m_resources->name() == "GL")
	{
		std::string text = std::string(vs_GL);
		std::regex to_replace("%NUM_VIEWS%");
		std::string patched_vs = std::regex_replace(text.c_str(), to_replace, std::to_string(view_matrices.size()));

		return { patched_vs, gs_GL, fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}

void RP501::init(RenderResources* r)
{
	m_resources = r;

	m_textures = { std::vector<int>(1) };
	m_renderbuffers = { std::vector<int>(1) };
	m_framebuffers = { std::vector<int>(1) };

	RenderResources::ArrayTextureConfig c_config;
	c_config.width = m_width;
	c_config.height = m_height;
	c_config.num_layers = (uint32_t)view_matrices.size();
	c_config.format = RenderResources::TextureFormat::R32UI;

	RenderResources::ArrayTextureConfig d_config(m_width, m_height, (uint32_t)view_matrices.size(), RenderResources::TextureFormat::DEPTH);
	RenderResources::FramebufferConfig f_config;

	m_textures[0][0] = m_resources->buildArrayTexture(c_config);
	m_renderbuffers[0][0] = m_resources->buildArrayTexture(d_config);
	m_framebuffers[0][0] = m_resources->buildFramebuffer(f_config);

	// TODO: adapt this. Make a solution that ensures that even many views can be rendered, tho the limitation for MVR is currently 4.
	// Either solve it via multipass or in combination with other features..? Need a fallback method for running out of MVR views.
	// At least make security check!

	m_resources->combineLayeredMultiview(m_framebuffers[0][0], { m_textures[0][0] }, m_renderbuffers[0][0], (int)view_matrices.size());
	m_resources->enableTargets(m_framebuffers[0][0], { 0 });

	if (!m_resources->checkFramebuffer(m_framebuffers[0][0]))
	{
		throw std::runtime_error("Problem making framebuffer!");
	}
}