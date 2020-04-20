#pragma once

#include <stdexcept>
#include <vector>
#include "RenderResources.h"
#include "RenderResourcesGL.h"
#include "EvalConfig.h"
struct TechniqueConfig;

struct TextureAndLayer
{
	TextureAndLayer() : m_tex_name{-1}, m_layer{0}, m_is_array_texture{false} {}
	TextureAndLayer(int tex_name, int layer, bool isArrayTex) : m_tex_name{tex_name}, m_layer{layer}, m_is_array_texture{isArrayTex} {}
	int m_tex_name;
	int m_layer;
	bool m_is_array_texture;
};


class RenderPipeline
{
public:
	RenderPipeline(std::string name, uint32_t width, uint32_t height, GeometryProviderBase& geometry);
	virtual void init(RenderResources* resources) = 0;
	virtual void begin() = 0;
	virtual void produce() = 0;
	virtual void end() {};
	virtual void dumpImages(const char* directory) = 0;
	virtual void buildProgram();

	virtual int get_view_matrices_count() const { throw std::runtime_error("not implemented"); }
	virtual glm::mat4 get_view_proj_matrix(int i) { throw std::runtime_error("not implemented"); }
	virtual glm::mat4 get_view_matrix(int i) { throw std::runtime_error("not implemented"); }
	virtual glm::mat4 get_proj_matrix(int i) { throw std::runtime_error("not implemented"); }
	virtual const std::vector<glm::mat4>& get_all_view_proj_matrices() { throw std::runtime_error("not implemented"); }
	virtual const glm::mat4* get_view_proj_matrices_data_ptr() { throw std::runtime_error("not implemented"); }
	virtual TextureAndLayer get_nth_result_texture_info(int index, int range = 0) { return {}; }
	virtual TextureAndLayer get_nth_depth_texture_info(int index, int range = 0) { return {}; }
	virtual glm::ivec2 get_nth_result_offsets(int index) { return { 0, 0 }; }
	virtual glm::ivec2 get_nth_result_range(int index) { return { static_cast<int>(m_width), static_cast<int>(m_height) }; }

	virtual bool activate_next_set() { throw std::runtime_error("not implemented"); }
	virtual bool activate_prev_set() { throw std::runtime_error("not implemented"); }

	std::string name() const { return m_name + "{" + m_geometry.get().name() + "}"; }
	auto program_handle() const { return m_program; }
	auto width() const { return m_width; }
	auto height() const { return m_height; }
	auto widthf() const { return static_cast<float>(m_width); }
	auto heightf() const { return static_cast<float>(m_height); }

	// Looks like textures, renderbuffers, and framebuffers are organized in... "ranges"?!
	// Who knows the intention behind this? No idea. Let's just call it "ranges"!
	std::vector<int>& textures_for_range(size_t range);
	std::vector<int>& renderbuffers_for_range(size_t range);
	std::vector<int>& framebuffers_for_range(size_t range);

	RenderResourcesGL* resources() const 
	{ 
		return static_cast<RenderResourcesGL*>(m_resources); 
	}

	void set_geometry_provider(GeometryProviderBase& geometry)
	{
		m_geometry = std::ref(geometry);
	}

	GeometryProviderBase& geometry_provider()
	{
		return m_geometry;
	}

	virtual EvalConfig& get_eval_config() { throw std::runtime_error("not implemented"); }
	virtual TechniqueConfig& get_technique_config() { throw std::runtime_error("not implemented"); }
	
protected:
	RenderResources* m_resources;
	std::reference_wrapper<GeometryProviderBase> m_geometry;

	std::vector<std::vector<int>> m_textures;
	std::vector<std::vector<int>> m_renderbuffers;
	std::vector<std::vector<int>> m_framebuffers;

	int m_program;
	bool m_built = false;
	uint32_t m_width, m_height;

	std::string m_name;

	virtual std::array<std::string, 3> getShaders() = 0;
};