#pragma once

#include <vector>
#include <glm/matrix.hpp>
#include <string>
#include <glad/glad.h>
//#include <GL/glew.h>

struct GeometryRenderInfo;
class GeometryProviderBase;
struct SceneObject;

enum struct duplication_strategy
{
	interleaved,
	consecutive
};

class RenderResources
{
protected:

	uint32_t current_matrix;
	uint32_t current_object;

	std::vector<const SceneObject*> scene_objects;

public:

	//struct IndirectDrawing
	//{
	//	int count;
	//	int instances;
	//	int offset;
	//	int base_vert;
	//};

	typedef  struct {
	    GLuint  count;
	    GLuint  instanceCount;
	    GLuint  firstIndex;
	    GLuint  baseVertex;
	    GLuint  baseInstance;
	} DrawElementsIndirectCommand;

	struct RenderbufferConfig
	{
		RenderbufferConfig() {}
		RenderbufferConfig(uint32_t width, uint32_t height) : width(width), height(height) {}
		uint32_t width, height;
	};

	enum class BufferBinding { DRAW_INDIRECT };

	enum class TextureFormat { R32UI, RGBA8, DEPTH };

	struct BufferConfig
	{
		enum class Usage { STREAM_DRAW, STATIC_DRAW, DYNAMIC_DRAW };
		BufferConfig() {}
		BufferConfig(size_t size, Usage usage) : size(size), usage(usage) {}
		size_t size;
		Usage usage;
	};

	struct ArrayTextureConfig
	{
		ArrayTextureConfig() {}
		ArrayTextureConfig(uint32_t width, uint32_t height, uint32_t layers, TextureFormat format)
			: width(width), height(height), num_layers(layers), format(format)
		{}
		uint32_t width, height, num_layers;
		TextureFormat format;
	};

	struct TextureConfig
	{
		TextureConfig() {}
		TextureConfig(uint32_t width, uint32_t height, TextureFormat format) : width(width), height(height), format(format) {}
		uint32_t width, height;
		TextureFormat format;
	};

	struct ViewportConfig
	{
		float x, y;
		float width, height;
	};

	struct FramebufferConfig
	{
	};

	enum class UniformType { MATRIX4x4, VEC2, INT, IVEC2 };

	struct UniformDeclaration
	{
		std::string name;
		UniformType type;
		bool array = false;
	};

	// This method returns some (seemingly arbitrary) scene object based on some internal counters.
	const SceneObject* getObject();

	// This method returns some (seemingly arbitrary) matrix of some (seemingly arbitrary) scene object based on some internal counters.
	const glm::mat4& getMatrix();

	// Advances some internal counters.
	bool advanceObjectMatrix();

	// Advance some internal object counter.
	bool advanceObject();

	virtual void buildMarkedPrimitivesBuffer(GeometryProviderBase& geometryProvider) = 0;
	virtual void bindMarkedPrimitivesBuffer(GLuint indexWhereToBind) = 0;
	virtual size_t getNumberOfMarkedPrimitives() = 0;
	
	/*
		Building resoucres needed for rendering	
	*/
	virtual int buildBuffer(BufferConfig config) = 0;

	virtual int buildIndirectsBuffer(BufferConfig::Usage usage, std::vector<DrawElementsIndirectCommand>& indirects) = 0;

	virtual int buildRenderbuffer(RenderbufferConfig config) = 0;

	virtual int buildTexture(ArrayTextureConfig config) = 0;

	virtual int buildArrayTexture(ArrayTextureConfig config) = 0;

	virtual int buildFramebuffer(FramebufferConfig config) = 0;

	virtual int buildProgram(const std::string& comp) = 0;

	virtual int buildProgram(const std::string& vs, const std::string& gs, const std::string& fs, GeometryProviderBase& geometry_provider) = 0;

	/*
	Handling uniforms
	*/

	virtual void setUniform(UniformType type, const char* name, const void* data) = 0;

	virtual void bindUniformBufferRange(uint32_t ubo_name, const char* buffer_decl, uint32_t size) = 0;

	// TODO: This should not be GL-specific code.. but.. you know..
	virtual void bindImageTexture(int tex_name, GLuint unit, GLint level = 0, GLint layer = 0, GLenum access = GL_READ_ONLY) = 0;

	/*
		Misc
	*/
	virtual void clearPrimitiveEnabledBuffer(uint32_t value) = 0;

	virtual void setIndirect(int buffer_name) = 0;

	virtual void bufferData(int buffer_name, uint32_t offset, const void* data, uint32_t data_size) = 0;

	virtual void combine(int fb_name, const std::vector<int>& cb_names, int db_name) = 0;

	virtual void combineLayered(int fb_name, const std::vector<int>& cb_names, int db_name) = 0;

	virtual void combineLayeredMultiview(int fb_name, const std::vector<int>& cb_names, int db_name, int num_views = 0) = 0;

	virtual void enableTargets(int fb_name, const std::vector<uint32_t>& targets) = 0;

	virtual void fetchTextureLayer(int tex_name, uint32_t layer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, std::vector<uint32_t>& data) = 0;

	virtual void fetchTextureLayer(int tex_name, uint32_t layer, std::vector<uint32_t>& data) = 0;

	virtual void activateFramebuffer(int fb_name) = 0;

	virtual void activateViewports(const std::vector<ViewportConfig>& viewports) = 0;

	virtual bool checkFramebuffer(int fb_name) = 0;

	/*
		Drawing, Syncing, Clearing
	*/

	virtual void clear() = 0;

	virtual void use(int prog_name) = 0;

	virtual void getWorkgroupSizes(int prog_name, GLint* out) = 0;

	virtual void begin(int prog_name, int duplication = 1, duplication_strategy dup_strategy = duplication_strategy::interleaved) = 0;

	virtual void renderAllInstanced(size_t num_instances, GeometryProviderBase& geometry_provider, std::vector<glm::mat4> viewProjMatrices) = 0;

	virtual void renderAllMulti(size_t num_draws, GeometryProviderBase& geometry_provider, std::vector<glm::mat4> viewProjMatrices) = 0;

	virtual void renderAll(GeometryProviderBase& geometry_provider, std::vector<glm::mat4> viewProjMatrices) = 0;

	virtual void renderCurrent() = 0;

	virtual void renderIndirectMulti(size_t multi, GeometryRenderInfo renderInfo) = 0;

	virtual void renderCurrentInstanced() = 0;

	virtual void sync() = 0;

	/*
		Description
	*/

	virtual const char* name() = 0;

	RenderResources(const SceneObject* objects, uint32_t num_objects);

	virtual ~RenderResources()
	{}
};