#include "RenderPipeline.h"
#include "RenderResources.h"
#include <array>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

RenderPipeline::RenderPipeline(std::string name, uint32_t width, uint32_t height, GeometryProviderBase& geometry)
	: m_program { -1 }
	, m_name{ name }
	, m_width{ width }
	, m_height{ height }
	, m_geometry{ std::ref(geometry) }
{
}

std::vector<int>& RenderPipeline::textures_for_range(size_t range)
{
	while (m_textures.size() <= range)
	{
		m_textures.emplace_back();
	}
	return m_textures[range];
}

std::vector<int>& RenderPipeline::renderbuffers_for_range(size_t range)
{
	while (m_renderbuffers.size() <= range)
	{
		m_renderbuffers.emplace_back();
	}
	return m_renderbuffers[range];
}

std::vector<int>& RenderPipeline::framebuffers_for_range(size_t range)
{
	while (m_framebuffers.size() <= range)
	{
		m_framebuffers.emplace_back();
	}
	return m_framebuffers[range];
}

void RenderPipeline::buildProgram()
{
	if (-1 != m_program) {
		throw std::runtime_error("Program has already been built");
	}
	auto shaders = getShaders();
	m_program = m_resources->buildProgram(shaders[0], shaders[1], shaders[2], geometry_provider());
}