#pragma once

#include "RenderResources.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <GL/GL.h>
#include <vector>
#include "GeometryProvider.h"

struct SceneVertex;
struct SceneObject;

class RenderResourcesGL : public RenderResources
{

public:

	bool makeHeavy = false;

	RenderResourcesGL(
		const std::vector<SceneObject>& objects);

	virtual ~RenderResourcesGL();

	
	virtual void buildMarkedPrimitivesBuffer(GeometryProviderBase& geometryProvider) override;

	virtual void bindMarkedPrimitivesBuffer(GLuint indexWhereToBind) override;

	virtual size_t getNumberOfMarkedPrimitives() override;

	virtual int buildBuffer(BufferConfig config) override;

	void deleteBuffer(int bufferId);

	virtual int buildIndirectsBuffer(BufferConfig::Usage usage, std::vector<DrawElementsIndirectCommand>& indirects) override;

	virtual int buildTexture(ArrayTextureConfig config) override;

	virtual int buildArrayTexture(ArrayTextureConfig config) override;

	virtual int buildRenderbuffer(RenderbufferConfig config) override;

	virtual int buildFramebuffer(FramebufferConfig config) override;

	virtual int buildProgram(const std::string& comp) override;

	virtual int buildProgram(const std::string& vs, const std::string& gs, const std::string& fs, GeometryProviderBase& geometry_provider) override;

	virtual void clearPrimitiveEnabledBuffer(uint32_t value) override;

	virtual void setIndirect(int buffer_name);

	virtual void bufferData(int buffer_name, uint32_t offset, const void* data, uint32_t data_size) override;
	
	virtual void bindUniformBufferRange(uint32_t ubo_name, const char* buffer_decl, uint32_t size) override;

	virtual void bindImageTexture(int tex_name, GLuint unit, GLint level = 0, GLint layer = 0, GLenum access = GL_READ_ONLY) override;

	virtual void setUniform(UniformType type, const char* name, const void* data) override;

	virtual void combine(int fb_name, const std::vector<int>& cb_names, int db_name) override;

	virtual void combineLayered(int fb_name, const std::vector<int>& cb_names, int db_name) override;

	virtual void combineLayeredMultiview(int fb_name, const std::vector<int>& cb_names, int db_name, int num_views) override;

	virtual void activateFramebuffer(int fb_name);

	void activateViewport(ViewportConfig viewport);

	virtual void activateViewports(const std::vector<ViewportConfig>& viewports);

	virtual bool checkFramebuffer(int fb_name);

	virtual void enableTargets(int fb_name, const std::vector<uint32_t>& targets) override;

	virtual void fetchTextureLayer(int tex_name, uint32_t layer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, std::vector<uint32_t>& data) override;

	virtual void fetchTextureLayer(int tex_name, uint32_t layer, std::vector<uint32_t>& data) override;

	RenderResources::ArrayTextureConfig getTextureConfig(int tex_name);

	virtual void clear() override;

	virtual void use(int prog_name) override;

	virtual void getWorkgroupSizes(int prog_name, GLint* out) override;

	virtual GLuint get_gl_handle(int prog_name);

	/**
	 *	@param	prog_name		Shader program handle
	 *	@param	duplication		Creates a blown-up index buffer which simply multiplies the amount of geometry that is submitted
	 *							If the m_duplication value does not match the previous m_duplication value, the index buffer will be modified.
	 */
	virtual void begin(int prog_name, int duplication=1, duplication_strategy dup_strategy = duplication_strategy::interleaved) override;

	virtual void renderAll(GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices) override;

	virtual void renderAllInstanced(size_t num_instances, GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices) override;

	virtual void renderAllMulti(size_t num_draws, GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices) override;

	virtual void renderIndirectMulti(size_t multi, GeometryRenderInfo renderInfo) override;

	virtual void renderCurrent() override;

	virtual void renderCurrentInstanced() override;	

	virtual void sync() override;

	virtual const char* name() 
	{
		return "GL";
	}

	GLuint getTextureHandle(int texName) { return m_textures[texName]; }
	GLuint getProgramHandle(int progName) { return m_programs[progName]; }

private:

	void cleanupScene();

	void cleanupPipeline();

	GLuint translateFormat(TextureFormat format);

	void translateFormat(TextureFormat t_format, GLuint& c_format, GLuint& c_type);


	int m_duplication = 1;

	GLuint m_primitive_enabled_buffer;
	
	//Mode data
	std::vector<GLuint> m_programs;
	std::vector<GLuint> m_shaders;

	std::vector<GLuint> m_framebuffers;
	std::vector<GLuint> m_textures;
	std::vector<ArrayTextureConfig> m_texture_configs;
	std::vector<GLuint> m_buffers;
	std::vector<GLuint> m_renderbuffers;

	size_t m_numMarkablePrimitives;
};