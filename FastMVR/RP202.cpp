#include "RP202.h"
#include <string>
#include "RenderResources.h"
#include <array>
#include <regex>
#include <stb_image_write.h>

// RP202
// Can only render the entire scene at once.
// Can only render the entire depth range at once.
// Assumes that model matrices are baked into the geometry.
// Uses an array texture and geometry shader to select layer for each view point rendering.
// Geometry shader duplicates geometry in loop.
// Uses a uniform buffer to set all view matrices at once.

#pragma region ShaderCode

const char* RP202::vs_GL = R"(
#version 450

in layout(location = 0) vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0f);
}
)";

const char* RP202::gs_GL = R"(
#version 450

layout(std140) uniform Vs
{
	mat4 V[32];
};

layout(triangles) in;
layout(triangle_strip, max_vertices = %NUM_VIEWS% * 3) out;

void main()
{
	const int num_views = %NUM_VIEWS%;
	for(int i = 0; i < num_views; i++)
	{
		gl_Layer = i;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[i] * gl_in[0].gl_Position;
		EmitVertex();
		gl_Layer = i;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[i] * gl_in[1].gl_Position;
		EmitVertex();
		gl_Layer = i;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[i] * gl_in[2].gl_Position;
		EmitVertex();	
		EndPrimitive();
	}
}
)";

const char* RP202::fs_GL = R"(
#version 450
out layout(location = 0) uint d_color;

void main()
{
	d_color = gl_PrimitiveID;
}
)";

#pragma endregion

RP202::RP202(
	uint32_t width,
	uint32_t height,
	std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry)
	:
	ViewTechnique("later...", width, height, view_matrices, geometry)
{
	m_name = "RP202";
}

std::array<std::string, 3> RP202::getShaders()
{
	if (m_resources->name() == "GL")
	{
		std::string text = std::string(gs_GL);
		std::regex to_replace("%NUM_VIEWS%");
		std::string patched_gs = std::regex_replace(text.c_str(), to_replace, std::to_string(view_matrices.size()));

		return { vs_GL, patched_gs, fs_GL };
	}
	else
	{
		throw std::runtime_error("Unknown resource!");
	}
}

void RP202::init(RenderResources* r)
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

	m_resources->combineLayered(m_framebuffers[0][0], {m_textures[0][0]}, m_renderbuffers[0][0]);
	m_resources->enableTargets(m_framebuffers[0][0], { 0 });

	if (!m_resources->checkFramebuffer(m_framebuffers[0][0]))
	{
		throw std::runtime_error("Problem making framebuffer!");
	}
}

void RP202::begin()
{
	// Make sure program is bound and uniforms can be reached
	// Needs baked geometry, has no concept of matrices, therefore parameter "baked" is true.
	m_resources->begin(m_program, true);

	// Update uniform buffer with view matrix data.
	RenderResources::BufferConfig conf(sizeof(glm::mat4) * (uint32_t)view_matrices.size(), RenderResources::BufferConfig::Usage::STREAM_DRAW);
	int buff = m_resources->buildBuffer(conf);
	m_resources->bufferData(buff, 0, view_matrices.data(), sizeof(glm::mat4) * (uint32_t)view_matrices.size());
	m_resources->bindUniformBufferRange(buff, "Vs", sizeof(glm::mat4) * (uint32_t)view_matrices.size());

	// We only have one framebuffer, let's activate it before rendering.
	m_resources->activateFramebuffer(m_framebuffers[0][0]);
}

void RP202::start()
{
	m_resources->clear();
	m_resources->renderAll(geometry_provider(), view_matrices);
}

void RP202::dumpImages(const char* directory)
{
	uint32_t w = m_width;
	uint32_t h = m_height;
	std::vector<uint32_t> data(m_width*m_height);

	for (int l = 0; l < view_matrices.size(); l++)
	{
		m_resources->fetchTextureLayer(m_textures[0][0], l, data);
		for (int i = 0; i < data.size(); i++)
		{
			data[i] |= 0xFF000000; // set full alpha. hack! remove later!
		}
		std::string name = std::string(directory) + std::string("v") + std::to_string(l) + std::string("_d") + std::to_string(0) + std::string(".png");
		stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
	}
}