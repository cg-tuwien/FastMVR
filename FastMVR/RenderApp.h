#pragma once
#include <glad/glad.h>
//#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "RenderPipeline.h"
#include <array>
#include "RenderResourcesGL.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "Helper.h"
#include "string_utils.h"
#include "ComputePipeline.h"
#include "GenericTechnique.h"

class RenderResources;

struct GLFWwindow;

class RenderToScreen : public RenderPipeline
{
public:
	RenderToScreen(uint32_t width, uint32_t height, RenderResourcesGL* iHaveNoIdeaWhatImDoing, GeometryProviderBase& geometry) 
		: RenderPipeline("RenderToScreen", width, height, geometry)
	{
		m_resources = iHaveNoIdeaWhatImDoing;
	}

	void init(RenderResources* resources) override
	{
	}

	void begin() override
	{
		// Into the back buffer:
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width(), height());
		glClearColor(0.066f, 0.135f, 0.24f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_resources->begin(program_handle(), true);
	}

	void produce() override
	{
		std::cout << "I'm not a producer. render() me plz!" << std::endl;
	}

	void render(glm::mat4 vM, glm::mat4 pM)
	{
		m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "V", glm::value_ptr(vM));
		m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "P", glm::value_ptr(pM));
		int primitiveIdMappingEnabled = m_forcePrimitiveIdMappingOff
				? 0
				: (geometry_provider().requiresPrimitiveIdMapping() ? 1 : 0);
		m_resources->setUniform(RenderResources::UniformType::INT, "uPrimitiveIdMappingEnabled", &primitiveIdMappingEnabled);
		m_resources->bindMarkedPrimitivesBuffer(2); // layout(std430, binding = 2) buffer PrimitiveEnabledBuffer
		m_resources->clear();
		m_resources->renderAll(geometry_provider(), {});
	}

	void setForcePrimitiveIdMappingOff(bool forceOff) { m_forcePrimitiveIdMappingOff = forceOff; }

	void dumpImages(const char* directory) override {}
	std::array<std::string, 3> getShaders() override
	{
#ifdef SHADOW_MAPPING
		auto vert = from_file("render_textured_screen_quad.vert");
		auto frag = from_file("render_textured_screen_quad.frag");
#ifdef GBUFFER
		vert = insert_into_shader_code_after_version(vert, "#define GBUFFER");
		frag = insert_into_shader_code_after_version(frag, "#define GBUFFER");
#endif
		return {vert , "",  frag};
#else
		return { from_file("render_to_screen.vert"), "", from_file("render_to_screen.frag") };
#endif
	}

	bool m_forcePrimitiveIdMappingOff;
};

//class FrustumRenderer : public RenderPipeline
//{
//public:
//	FrustumRenderer(uint32_t width, uint32_t height, RenderResourcesGL* iHaveNoIdeaWhatImDoing, GeometryProviderBase& geometry) 
//		: RenderPipeline("RenderToScreen", width, height, geometry)
//	{
//		m_resources = iHaveNoIdeaWhatImDoing;
//	}
//
//	void init(RenderResources* resources) override
//	{
//	}
//
//	void begin() override
//	{
//		// Into the back buffer:
//		m_resources->begin(program_handle(), true);
//	}
//
//	void produce() override
//	{
//		std::cout << "I'm not a producer. render() me plz!" << std::endl;
//	}
//
//	void render(glm::mat4 vM, glm::mat4 pM)
//	{
//		m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "V", glm::value_ptr(vM));
//		m_resources->setUniform(RenderResources::UniformType::MATRIX4x4, "P", glm::value_ptr(pM));
//		m_resources->clear();
//		m_resources->renderAll(geometry_provider(), {});
//	}
//
//	void setForcePrimitiveIdMappingOff(bool forceOff) { m_forcePrimitiveIdMappingOff = forceOff; }
//
//	void dumpImages(const char* directory) override {}
//	std::array<std::string, 3> getShaders() override
//	{
//		return { from_file("render_to_screen.vert"), "", from_file("render_to_screen.frag") };
//	}
//
//	bool m_forcePrimitiveIdMappingOff;
//};


class MarkVisiblePrimitives : public ComputePipeline
{
public:
	MarkVisiblePrimitives(RenderResourcesGL* resourcesDoEverything) 
		: ComputePipeline( from_file("mark_visible_primitives.comp") )
	{
		ComputePipeline::init(resourcesDoEverything);
	}

	void bindResources() override
	{
		m_resources->bindImageTexture(m_tex_and_layer.m_tex_name, /* unit: */ 1, /* level: */ 0, m_tex_and_layer.m_layer);
		m_resources->bindMarkedPrimitivesBuffer(2); // layout(std430, binding = 2) buffer PrimitiveEnabledBuffer
		m_resources->setUniform(RenderResources::UniformType::IVEC2, "uOffset", glm::value_ptr(m_offset));
		m_resources->setUniform(RenderResources::UniformType::IVEC2, "uRange", glm::value_ptr(m_range));
	}

	void markAllFromTexture(TextureAndLayer texInfo, glm::ivec2 offset, glm::ivec2 range)
	{
		m_tex_and_layer = texInfo;
		m_offset = offset;
		m_range = range;
		compute(m_range.x, m_range.y);
	}

private:
	TextureAndLayer m_tex_and_layer;
	glm::ivec2 m_offset;
	glm::ivec2 m_range;

};

class VisiblePrimitivesCounter : public ComputePipeline
{
public:
	VisiblePrimitivesCounter(RenderResourcesGL* resourcesDoEverything) 
		: ComputePipeline( from_file("count_marked_primitives.comp") )
	{
		ComputePipeline::init(resourcesDoEverything);
	}

	void bindResources() override
	{
		m_resources->bindMarkedPrimitivesBuffer(2); // layout(std430, binding = 2) buffer PrimitiveEnabledBuffer
	}

	void computeCount()
	{
		compute(m_resources->getNumberOfMarkedPrimitives());
	}

};

enum struct measurement_type { gpu_time, cpu_time };
enum struct load_config { nothing, heavy_vertex_load };

// Sort of the "main-thing"
class RenderApp
{
public:
	// Create with given dimensions
	RenderApp(int initialWidth, int initialHeight, GeometryProviderBase& geometryProviderForRendering);

	void setResources(RenderResourcesGL* newResources);

	void setGeometryForRendering(GeometryProviderBase& geometryProvider) { geometryForRendering = std::ref(geometryProvider); }
	void addAdditionalGeometryProviders(GeometryProviderBase& geometryProvider) { additionalGeometryProviders.push_back(std::ref(geometryProvider)); }
	
	inline void setResultsOutputDirectory(std::string pathToDirectory) { m_resultsOutputDirectory = std::move(pathToDirectory); }
	inline const auto& path_to_results_output_directory() const { return m_resultsOutputDirectory; }

	void run(std::vector<RenderPipeline*> pipelines, measurement_type measurementType, size_t startIndex = 0, bool automaticModeOn = false, int numberOfMeasurementsInAutomaticMode = 3);

	~RenderApp();
private:
	int width, height;
	RenderResourcesGL* resources;
	std::reference_wrapper<GeometryProviderBase> geometryForRendering;
	std::vector<std::reference_wrapper<GeometryProviderBase>> additionalGeometryProviders;
	GLFWwindow* window;
	std::string m_resultsOutputDirectory;
};