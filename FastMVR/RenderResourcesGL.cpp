#include <glad/glad.h>
//#include <GL/glew.h>
#include "RenderResourcesGL.h"
#include <iostream>
#include "GLHelper.h"
#include <algorithm>
#include "FileHelper.h"
#include "GeometryProviderEverything.h"

#define GL_SCENE
#include "Scene.h"
#include "GeometryProviderCubified.h"
#include "string_utils.h"
#undef GL_SCENE


GLuint RenderResourcesGL::translateFormat(TextureFormat t_format)
{
	switch (t_format)
	{
	case TextureFormat::RGBA8:
		return GL_RGBA8;
	case TextureFormat::DEPTH:
		return GL_DEPTH_COMPONENT32F;
	case TextureFormat::R32UI:
		return GL_R32UI;
	default:
		throw std::runtime_error("Unsupported format!");
		return -1;
	}
}

void RenderResourcesGL::translateFormat(TextureFormat t_format, GLuint& c_format, GLuint& c_type)
{
	switch (t_format)
	{
	case TextureFormat::RGBA8:
		c_format = GL_RGBA;
		c_type = GL_BYTE;
		break;
	case TextureFormat::R32UI:
		c_format = GL_RED_INTEGER;
		c_type = GL_UNSIGNED_INT;
		break;
	default:
		throw std::runtime_error("Unsupported format!");
		break;
	}
}

RenderResourcesGL::RenderResourcesGL(
		const std::vector<SceneObject>& objects
)
	: RenderResources(objects.data(), objects.size())
{
	GLint param = 0;
	//std::cout << "We have" << (GLEW_OVR_multiview && GLEW_OVR_multiview2 ? " " : " NO ") << "support for multi-view rendering" << std::endl;
	std::cout << "\n";
	std::cout << "We have" << (GLAD_GL_OVR_multiview || GLAD_GL_OVR_multiview2 ? " " : " NO ") << "support for multi-view rendering" << std::endl;
	std::cout << "  GL_OVR_multiview is" << (GLAD_GL_OVR_multiview ? " " : " NOT ") << "supported" << std::endl;
	std::cout << "  GL_OVR_multiview2 is" << (GLAD_GL_OVR_multiview2 ? " " : " NOT ") << "supported" << std::endl;
	std::cout << "\n";
	std::cout << "We have" << (GLAD_GL_NV_representative_fragment_test ? " " : " NO ") << "support for representative fragment testing" << std::endl;
	std::cout << "  GL_NV_representative_fragment_test is" << (GLAD_GL_NV_representative_fragment_test ? " " : " NOT ") << "supported" << std::endl;
	std::cout << "\n";

	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &param));
	std::cout << "Stats:" << std::endl;
	std::cout << "\tMax texture size = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_VIEWPORTS, &param));
	std::cout << "\tMax number viewports = " << param << std::endl;
	std::cout << std::endl;

	GL_CALL(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &param));
	std::cout << "\tGL_MAX_UNIFORM_BUFFER_BINDINGS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &param));
	std::cout << "\tGL_MAX_UNIFORM_BLOCK_SIZE = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &param));
	std::cout << "\tGL_MAX_VERTEX_UNIFORM_BLOCKS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &param));
	std::cout << "\tGL_MAX_FRAGMENT_UNIFORM_BLOCKS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &param));
	std::cout << "\tGL_MAX_GEOMETRY_UNIFORM_BLOCKS= " << param << std::endl;

	GL_CALL(glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &param));
	std::cout << "\tGL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &param));
	std::cout << "\tGL_MAX_SHADER_STORAGE_BLOCK_SIZE = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &param));
	std::cout << "\tGL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &param));
	std::cout << "\tGL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, &param));
	std::cout << "\tGL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = " << param << std::endl;

	GL_CALL(glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &param));
	std::cout << "\tGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS = " << param << std::endl;
	GL_CALL(glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &param));
	std::cout << "\tGL_MAX_GEOMETRY_OUTPUT_VERTICES = " << param << std::endl;

	scene_objects.resize(objects.size());
	for (uint32_t i = 0; i < objects.size(); i++)
	{
		scene_objects[i] = &objects[i];
	}
}

void RenderResourcesGL::buildMarkedPrimitivesBuffer(GeometryProviderBase& geometryProvider)
{
	auto geometry = geometryProvider.getGeometryForMatrices({});
	assert (geometry.size() == 1);
	
	GL_CALL(glGenBuffers(1, &m_primitive_enabled_buffer));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_primitive_enabled_buffer));
	const size_t num_primitives = geometry.front().m_drawcall_data.count / 3; // number of indices / 3 => Triangles only!
	m_numMarkablePrimitives = num_primitives;
	std::vector<uint32_t> primitive_enabled;
	primitive_enabled.reserve(num_primitives);
	for (size_t i = 0; i < num_primitives; ++i) {
		if (i < num_primitives / 10) {
			primitive_enabled.push_back(1); // enable first half of primitives
		}
		else {
			primitive_enabled.push_back(0); // disable second half of primitives
		}
	}
	GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, num_primitives * sizeof(uint32_t), primitive_enabled.data(), GL_DYNAMIC_DRAW));
	//GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 32, m_primitive_enabled_buffer));
}

void RenderResourcesGL::bindMarkedPrimitivesBuffer(GLuint indexWhereToBind)
{
	GL_CALL(
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexWhereToBind, m_primitive_enabled_buffer)
	);
}

size_t RenderResourcesGL::getNumberOfMarkedPrimitives()
{
	return m_numMarkablePrimitives;
}

void RenderResourcesGL::clearPrimitiveEnabledBuffer(uint32_t value)
{
	GL_CALL(glClearNamedBufferData(m_primitive_enabled_buffer, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &value));
}

void RenderResourcesGL::sync()
{
	GL_CALL(glFinish());
}

void RenderResourcesGL::clear()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void RenderResourcesGL::use(int prog_name)
{
	GLuint prog = m_programs[prog_name];
	GL_CALL(glUseProgram(prog));
}

void RenderResourcesGL::getWorkgroupSizes(int prog_name, GLint* out)
{
	GLuint prog = m_programs[prog_name];	
	GL_CALL(glGetProgramiv(prog, GL_COMPUTE_WORK_GROUP_SIZE, out));
}

GLuint RenderResourcesGL::get_gl_handle(int prog_name)
{
	return m_programs[prog_name];
}

void RenderResourcesGL::begin(int prog_name, int dup, duplication_strategy dup_strategy)
{
	if (dup != m_duplication)
	{
		//m_indices_to_draw = (uint32_t)(m_stored_indices.size() * dup);
		//std::vector<uint32_t> new_indices;

		//if (1 == dup) {
		//	new_indices = m_stored_indices;
		//}
		//else {
		//	switch (dup_strategy) {
		//	case duplication_strategy::interleaved:
		//		new_indices.resize(m_indices_to_draw);
		//		for (int i = 0; i < m_stored_indices.size(); i += 3)
		//		{
		//			for (int j = 0; j < dup; j++)
		//			{
		//				int off_d = i * dup + j * 3;
		//				for (int k = 0; k < 3; k++)
		//				{
		//					new_indices[off_d + k] = m_stored_indices[i + k];
		//				}
		//			}
		//		}
		//		break;
		//	case duplication_strategy::consecutive:
		//		new_indices.reserve(m_indices_to_draw);
		//		for (int i = 0; i < dup; ++i) {
		//			new_indices.insert(new_indices.end(), std::begin(m_stored_indices), std::end(m_stored_indices));
		//		}
		//		break;
		//	default:
		//		assert(false); throw std::runtime_error("wtf?!");
		//	}
		//}


		//GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer));
		//GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices_to_draw, new_indices.data(), GL_STATIC_DRAW));
		//m_duplication = dup;

		std::cout << "\nWARNING: index buffer duplication currently not active. Hasn't been adapted to GeometryProvider yet.\n" << std::endl;
	}

	GLuint prog = m_programs[prog_name];

	glUseProgram(prog);
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glClearColor(0, 0, 0, 1));
}

void RenderResourcesGL::renderAll(GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices)
{
	auto toRender = geometryProvider.getGeometryForMatrices(viewProjMatrices);
	for (auto& r : toRender) 
	{
		GL_CALL(glBindVertexArray(r.m_vao));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 0));
#ifdef GBUFFER
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 1));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 2));
#endif
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r.m_vertex_buffer));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.m_index_buffer));

		if (geometryProvider.requiresPrimitiveIdMapping()) 
		{
			assert(r.m_primitive_id_mapping_buffer != 0);
			//GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, r.m_primitive_id_mapping_buffer));
			GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.m_primitive_id_mapping_buffer)); // layout(std430, binding = 0) buffer PrimitiveIdMapping
			setUniform(UniformType::INT, "uPrimitiveIdMappingBufferOffset", &r.m_offset_into_primitive_id_mapping_buffer);
		}

		GL_CALL(
		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES,
			r.m_drawcall_data.count,
			GL_UNSIGNED_INT,
			reinterpret_cast<const void*>(r.m_drawcall_data.firstIndex),
			r.m_drawcall_data.instanceCount,
			r.m_drawcall_data.baseVertex,
			r.m_drawcall_data.baseInstance
		)
		);
	}
	
}

void RenderResourcesGL::renderAllMulti(size_t num_draws, GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices)
{
	auto toRender = geometryProvider.getGeometryForMatrices(viewProjMatrices);
	for (auto& r : toRender) 
	{
		GL_CALL(glBindVertexArray(r.m_vao));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 0));
#ifdef GBUFFER
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 1));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 2));
#endif
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r.m_vertex_buffer));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.m_index_buffer));
		
		std::vector<GLsizei> nums(num_draws, r.m_drawcall_data.count);
		std::vector<const void*> off(num_draws, reinterpret_cast<const void*>(r.m_drawcall_data.firstIndex));
		std::vector<GLint> baseverts(num_draws, r.m_drawcall_data.baseVertex);
		
		if (geometryProvider.requiresPrimitiveIdMapping()) 
		{
			assert(r.m_primitive_id_mapping_buffer != 0);
			//GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, r.m_primitive_id_mapping_buffer));
			GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.m_primitive_id_mapping_buffer)); // layout(std430, binding = 0) buffer PrimitiveIdMapping
			setUniform(UniformType::INT, "uPrimitiveIdMappingBufferOffset", &r.m_offset_into_primitive_id_mapping_buffer);
		}

		GL_CALL(
		glMultiDrawElementsBaseVertex(
			GL_TRIANGLES,
			nums.data(),
			GL_UNSIGNED_INT,
			off.data(),
			num_draws,
			baseverts.data()
		)
		);
	}
}

void RenderResourcesGL::renderAllInstanced(size_t num_instances, GeometryProviderBase& geometryProvider, std::vector<glm::mat4> viewProjMatrices)
{
	auto toRender = geometryProvider.getGeometryForMatrices(viewProjMatrices);
	for (auto& r : toRender) 
	{
		GL_CALL(glBindVertexArray(r.m_vao));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 0));
#ifdef GBUFFER
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 1));
		GL_CALL(glEnableVertexArrayAttrib(r.m_vao, 2));
#endif
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r.m_vertex_buffer));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.m_index_buffer));

		assert(r.m_drawcall_data.instanceCount == 1); // Don't know how to handle values other than 1
		
		if (geometryProvider.requiresPrimitiveIdMapping()) 
		{
			assert(r.m_primitive_id_mapping_buffer != 0);
			//GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, r.m_primitive_id_mapping_buffer));
			GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r.m_primitive_id_mapping_buffer)); // layout(std430, binding = 0) buffer PrimitiveIdMapping
			setUniform(UniformType::INT, "uPrimitiveIdMappingBufferOffset", &r.m_offset_into_primitive_id_mapping_buffer);
		}

		GL_CALL(
		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES,
			r.m_drawcall_data.count,
			GL_UNSIGNED_INT,
			reinterpret_cast<const void*>(r.m_drawcall_data.firstIndex),
			num_instances,
			r.m_drawcall_data.baseVertex,
			r.m_drawcall_data.baseInstance
		)
		);
	}
}

void RenderResourcesGL::renderIndirectMulti(size_t multi, GeometryRenderInfo renderInfo)
{
	// Attention: 
	// It's just ONE SINGLE draw call.
	// Everything must have been set up in advance.
	//
	// This method assumes that all the geometry is in ONE SINGLE BUFFER, so
	// only the buffer at index 0 will be bound. the rest of the config must
	// already be in place (via `buildIndirectsBuffer` and `setIndirect`) so
	// that a single draw call can be issued.
	//
	// All further GeometryRenderData instances after the first one will be
	// iterated just for the purpose of sanity checking.

	GL_CALL(glBindVertexArray(renderInfo.m_vao));
	GL_CALL(glEnableVertexArrayAttrib(renderInfo.m_vao, 0));
#ifdef GBUFFER
	GL_CALL(glEnableVertexArrayAttrib(renderInfo.m_vao, 1));
	GL_CALL(glEnableVertexArrayAttrib(renderInfo.m_vao, 2));
#endif
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderInfo.m_vertex_buffer));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.m_index_buffer));
	
	GL_CALL(
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, multi, 0)
	);
}

void RenderResourcesGL::renderCurrent()
{
	const SceneObject* s = scene_objects[current_object];
	uint32_t size = s->fromTo.y - s->fromTo.x;
	GL_CALL(glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, ((uint32_t*)0) + s->fromTo.x));
}

void RenderResourcesGL::renderCurrentInstanced()
{
	const SceneObject* s = scene_objects[current_object];
	uint32_t size = s->fromTo.y - s->fromTo.x;
	GL_CALL(
		glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, ((uint32_t*)0) + s->fromTo.x, (GLsizei)s->matrices.size())
	);
}

int RenderResourcesGL::buildProgram(const std::string& comp)
{
	GLenum shaderType = GL_COMPUTE_SHADER;
	GLuint shaderHandle = GL_CALL(glCreateShader(shaderType));
	assert(shaderHandle);
	const auto* source_c = static_cast<const GLchar*>(comp.c_str());
	GL_CALL(glShaderSource(shaderHandle, 1, &source_c, 0));
	GL_CALL(glCompileShader(shaderHandle));
	checkShader(shaderHandle, source_c);
	m_shaders.push_back(shaderHandle);

	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

	if (compileSuccess == GL_FALSE)
	{
		GLint infoLen = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen) 
		{
			char* buf = new char[infoLen];
			glGetShaderInfoLog(shaderHandle, infoLen, nullptr, buf);
			printf((std::string("Could not compile ") + (
				(shaderType == GL_VERTEX_SHADER) ? "vertex" :
				(shaderType == GL_TESS_CONTROL_SHADER) ? "tessellation control" :
				(shaderType == GL_TESS_EVALUATION_SHADER) ? "tessellation evaluation" :
				(shaderType == GL_GEOMETRY_SHADER) ? "geometry" :
				(shaderType == GL_FRAGMENT_SHADER) ? "fragment" :
				(shaderType == GL_COMPUTE_SHADER) ? "compute" : "?") + " shader with handle[%u]\n    Reason: %s\n").c_str(), shaderHandle, buf);
			delete[] buf;
		}

		glDeleteShader(shaderHandle);

		throw std::runtime_error("buildProgram");
	}

	GLuint progHandle = GL_CALL(glCreateProgram());
	GL_CALL(glAttachShader(progHandle, shaderHandle));

	GL_CALL(glLinkProgram(progHandle));

	GLint linkSuccess;
	glGetProgramiv(progHandle, GL_LINK_STATUS, &linkSuccess);

	if (linkSuccess == GL_FALSE)
	{
		printf("Linking shader program failed\n");

		GLint infoLen = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen) 
		{
			char* buf = new char[infoLen];
			glGetShaderInfoLog(shaderHandle, infoLen, nullptr, buf);
			printf((std::string("Could not link ") + (
				(shaderType == GL_VERTEX_SHADER) ? "vertex" :
				(shaderType == GL_TESS_CONTROL_SHADER) ? "tessellation control" :
				(shaderType == GL_TESS_EVALUATION_SHADER) ? "tessellation evaluation" :
				(shaderType == GL_GEOMETRY_SHADER) ? "geometry" :
				(shaderType == GL_FRAGMENT_SHADER) ? "fragment" :
				(shaderType == GL_COMPUTE_SHADER) ? "compute" : "?") + " shader with handle[%u]\n    Reason: %s\n").c_str(), shaderHandle, buf);
			delete[] buf;
		}
		
		throw std::runtime_error("Linking shader program failed");
	}

	checkProgram(progHandle);

	m_programs.push_back(progHandle);
	return (int)(m_programs.size() - 1);
}

static std::string VertLoadName("/vertex_load.glsl");
static std::string VertLoadCodeEasy(
R"(
vec3 SimulateVertexLoad(vec3 position)
{
	return position;
}
)"
);
static std::string VertLoadCodeHeavy(
R"(
vec3 SimulateVertexLoad(vec3 position)
{
	float fib1 = position.x;
	float fib2 = position.y;
	for(int i = 0; i < 1500; i++)
	{
		float t = fib1;
		fib1 = fib2;
		fib2 = t + fib2;
	}
	if(fib2 == 3.402823466e+38)
	{
		position.x += 1.0f;
	}
	return position;
}
)"
);

int RenderResourcesGL::buildProgram(const std::string& vs, const std::string& gs, const std::string& fs, GeometryProviderBase& geometry_provider)
{
	//This will become modular I guess
	std::string vs_source, fs_source, gs_source;
	vs_source = vs;
	gs_source = gs;
	fs_source = fs;

	if (!fs_source.empty() && geometry_provider.requiresPrimitiveIdMapping()) 
	{
		// Modify the fragment shader source code:
		fs_source = insert_into_shader_code_after_version(fs_source, "#define PRIMITIVE_ID_MAPPING_ENABLED");
	}

	const auto needle = std::string("#include \"/vertex_load.glsl\"");
	
	if (makeHeavy)
	{
		const auto pos = vs_source.find(needle);
		if (pos != std::string::npos) {
			vs_source.replace(pos, needle.size(), VertLoadCodeHeavy);
		}
		//GL_CALL(glNamedStringARB(GL_SHADER_INCLUDE_ARB, VertLoadName.length(), VertLoadName.c_str(), VertLoadCodeHeavy.length(), VertLoadCodeHeavy.c_str()));
	}
	else
	{
		const auto pos = vs_source.find(needle);
		if (pos != std::string::npos) {
			vs_source.replace(pos, needle.size(), VertLoadCodeEasy);
		}
		//GL_CALL(glNamedStringARB(GL_SHADER_INCLUDE_ARB, VertLoadName.length(), VertLoadName.c_str(), VertLoadCodeEasy.length(), VertLoadCodeEasy.c_str()));
	}
	
	const char* vs_source_c = vs_source.c_str(), * fs_source_c = fs_source.c_str(), * gs_source_c = gs_source.c_str();
	int vs_source_len = (int)vs_source.length(), fs_source_len = (int)fs_source.length(), gs_source_len = (int)gs_source.length();
	//This should stay the same

	GLuint vertex_shader = GL_CALL(glCreateShader(GL_VERTEX_SHADER));
	GL_CALL(glShaderSource(vertex_shader, 1, &vs_source_c, &vs_source_len));
	GL_CALL(glCompileShader(vertex_shader));
	checkShader(vertex_shader, vs_source_c);
	m_shaders.push_back(vertex_shader);

	GLuint program = GL_CALL(glCreateProgram());
	GL_CALL(glAttachShader(program, vertex_shader));

	if (fs_source_len > 2) 
	{
		GLuint fragment_shader = GL_CALL(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CALL(glShaderSource(fragment_shader, 1, &fs_source_c, &fs_source_len));
		GL_CALL(glCompileShader(fragment_shader));
		checkShader(fragment_shader, fs_source_c);
		m_shaders.push_back(fragment_shader);
		GL_CALL(glAttachShader(program, fragment_shader));
	}

	if (gs.length() > 2)
	{
		GLuint geometry_shader = GL_CALL(glCreateShader(GL_GEOMETRY_SHADER));
		GL_CALL(glShaderSource(geometry_shader, 1, &gs_source_c, &gs_source_len));
		GL_CALL(glCompileShader(geometry_shader));
		checkShader(geometry_shader, gs_source_c);
		m_shaders.push_back(geometry_shader);
		GL_CALL(glAttachShader(program, geometry_shader));
	}

	//glBindFragDataLocation(program, 0, "d_color");

	GL_CALL(glLinkProgram(program));
	checkProgram(program);

	m_programs.push_back(program);
	return (int)(m_programs.size() - 1);
}

int RenderResourcesGL::buildBuffer(BufferConfig config)
{
	uint32_t usage;
	switch (config.usage)
	{
	case BufferConfig::Usage::STREAM_DRAW:
		usage = GL_STREAM_DRAW;
		break;
	case BufferConfig::Usage::STATIC_DRAW:
		usage = GL_STATIC_DRAW;
		break;
	case BufferConfig::Usage::DYNAMIC_DRAW:
		usage = GL_DYNAMIC_DRAW;
		break;
	default:
		throw std::runtime_error("Unimplemented usage given!");
		break;
	}
	
	GLuint buffer;
	GL_CALL(glCreateBuffers(1, &buffer));
	GL_CALL(glNamedBufferData(buffer, config.size, 0, usage));
	m_buffers.push_back(buffer);
	return (int)(m_buffers.size() - 1);
}

void RenderResourcesGL::deleteBuffer(int bufferId)
{
	GL_CALL(
		glDeleteBuffers(1, &m_buffers[bufferId])
	);
	m_buffers[bufferId] = 0;
}

int RenderResourcesGL::buildIndirectsBuffer(BufferConfig::Usage usage, std::vector<DrawElementsIndirectCommand>& indirects)
{
	BufferConfig config;
	config.usage = usage;
	config.size = sizeof(DrawElementsIndirectCommand) * (uint32_t)(indirects.size());

	int buff = buildBuffer(config);

	bufferData(buff, 0, indirects.data(), sizeof(DrawElementsIndirectCommand) * indirects.size());

	return buff;
}

void RenderResourcesGL::setIndirect(int buffer_name)
{
	uint32_t buf = m_buffers[buffer_name];
	GL_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf));
}

void RenderResourcesGL::bufferData(int buffer_name, uint32_t offset, const void* data, uint32_t data_size)
{
	uint32_t buf = m_buffers[buffer_name];
	GL_CALL(glNamedBufferSubData(buf, offset, data_size, data));
}

int RenderResourcesGL::buildTexture(ArrayTextureConfig config)
{
	GLuint format = translateFormat(config.format);

	GLuint tex;
	GL_CALL(glGenTextures(1, &tex));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, tex));
	GL_CALL(glTextureStorage2D(tex, 1, format, config.width, config.height));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	m_textures.push_back(tex);
	m_texture_configs.push_back(config);
	return (int)(m_textures.size() - 1);
}

int RenderResourcesGL::buildArrayTexture(ArrayTextureConfig config)
{
	GLuint format = translateFormat(config.format);

	GLuint tex;
	GL_CALL(glGenTextures(1, &tex));
	GL_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, tex));
	GL_CALL(glTextureStorage3D(tex, 1, format, config.width, config.height, config.num_layers));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	m_textures.push_back(tex);
	m_texture_configs.push_back(config);
	return (int)m_textures.size() - 1;
}

int RenderResourcesGL::buildRenderbuffer(RenderbufferConfig config)
{
	GLuint rend;
	GL_CALL(glGenRenderbuffers(1, &rend));
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, rend));
	GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, config.width, config.height));
	m_renderbuffers.push_back(rend);
	return (int)m_renderbuffers.size() - 1;
}

int RenderResourcesGL::buildFramebuffer(FramebufferConfig config)
{
	GLuint fb;
	GL_CALL(glCreateFramebuffers(1, &fb));
	m_framebuffers.push_back(fb);
	return (int)m_framebuffers.size() - 1;
}

void RenderResourcesGL::activateFramebuffer(int fb_name)
{
	GLuint fb = m_framebuffers[fb_name];
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
}

void RenderResourcesGL::activateViewport(ViewportConfig viewport)
{
	GL_CALL(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
}

void RenderResourcesGL::activateViewports(const std::vector<ViewportConfig>& viewports)
{
	GL_CALL(glViewportArrayv(0, (GLsizei)viewports.size(), (float*)viewports.data()));
}


bool RenderResourcesGL::checkFramebuffer(int fb_name)
{
	GLuint fb = m_framebuffers[fb_name];
	auto code = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (code != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	return true;
}

void RenderResourcesGL::enableTargets(int fb_name, const std::vector<uint32_t>& targets)
{
	GLuint fb = m_framebuffers[fb_name];
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
	std::vector<GLenum> draw_buffers(targets.size());
	for (int i = 0; i < targets.size(); i++)
	{
		draw_buffers[i] = GL_COLOR_ATTACHMENT0 + targets[i];
	}
	GL_CALL(glDrawBuffers((GLsizei)targets.size(), draw_buffers.data()));
}

void RenderResourcesGL::combine(int fb_name, const std::vector<int>& cb_names, int db_name)
{
	assert(fb_name < m_framebuffers.size());
	GLuint fb = m_framebuffers[fb_name];
#ifdef SHADOW_MAPPING
	assert(db_name < m_textures.size()); // If it fails, maybe you have created a texture instead of a render buffer?
	GLuint db = m_textures[db_name];

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
	GL_CALL(
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, db, 0)
	);
#else
	assert(db_name < m_renderbuffers.size()); // If it fails, maybe you have created a texture instead of a render buffer?
	GLuint db = m_renderbuffers[db_name];

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
	GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, db));
#endif

	for (int i = 0; i < cb_names.size(); i++)
	{
		GLuint cb = m_textures[cb_names[i]];
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, cb, 0));
	}
}

void RenderResourcesGL::combineLayered(int fb_name, const std::vector<int>& cb_names, int db_name)
{
	assert(fb_name < m_framebuffers.size());
	GLuint fb = m_framebuffers[fb_name];
	assert(db_name < m_textures.size());
	GLuint db = m_textures[db_name];
	GL_CALL(glNamedFramebufferTexture(fb, GL_DEPTH_ATTACHMENT, db, 0));

	for (int i = 0; i < cb_names.size(); i++)
	{
		assert(cb_names[i] < m_textures.size());
		GLuint cb = m_textures[cb_names[i]];
		GL_CALL(glNamedFramebufferTexture(fb, GL_COLOR_ATTACHMENT0 + i, cb, 0));
	}
}

void RenderResourcesGL::combineLayeredMultiview(int fb_name, const std::vector<int>& cb_names, int db_name, int num_views)
{
	assert(fb_name < m_framebuffers.size());
	GLuint fb = m_framebuffers[fb_name];
	assert(db_name < m_textures.size());
	GLuint db = m_textures[db_name];
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
	GL_CALL(
		glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, db, 0, 0, num_views)
	);

	for (int i = 0; i < cb_names.size(); i++)
	{
		assert(cb_names[i] < m_textures.size());
		GLuint cb = m_textures[cb_names[i]];
		GL_CALL(
			glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, cb, 0, 0, num_views)
		);
	}
}

void RenderResourcesGL::fetchTextureLayer(int tex_name, uint32_t layer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, std::vector<uint32_t>& data)
{
	GLuint tex = m_textures[tex_name];
	GLuint c_format, c_type;
	ArrayTextureConfig config = m_texture_configs[tex_name];
	translateFormat(config.format, c_format, c_type);

	GL_CALL(glGetTextureSubImage(tex, 0, x, y, layer, w, h, 1, c_format, c_type, (GLsizei)data.size() * sizeof(uint32_t), data.data()));
}

void RenderResourcesGL::fetchTextureLayer(int tex_name, uint32_t layer, std::vector<uint32_t>& data)
{
	GLuint tex = m_textures[tex_name];
	GLuint c_format, c_type;

	ArrayTextureConfig config = m_texture_configs[tex_name];
	translateFormat(config.format, c_format, c_type);

	GL_CALL(glGetTextureSubImage(tex, 0, 0, 0, layer, config.width, config.height, 1, c_format, c_type, (GLsizei)data.size()*sizeof(uint32_t), data.data()));
}

RenderResources::ArrayTextureConfig RenderResourcesGL::getTextureConfig(int tex_name)
{
	return m_texture_configs[tex_name];
}

void RenderResourcesGL::bindUniformBufferRange(uint32_t ubo_name, const char* block_decl, uint32_t size)
{
	GLint id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);

	uint32_t ubo = m_buffers[ubo_name];
	uint32_t index = GL_CALL(glGetUniformBlockIndex(id, block_decl));
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
	GL_CALL(glBindBufferRange(GL_UNIFORM_BUFFER, index, ubo, 0, size));
}

void RenderResourcesGL::bindImageTexture(int tex_name, GLuint unit, GLint level, GLint layer, GLenum access)
{
	GLuint tex = m_textures[tex_name];
	ArrayTextureConfig config = m_texture_configs[tex_name];
	assert (layer < config.num_layers);

	// From https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindImageTexture.xhtml
	// layered: Specifies whether a layered texture binding is to be established.
	// layer:   If layered is GL_FALSE, specifies the layer of texture to be bound to the image unit. Ignored otherwise.
	glBindImageTexture(unit, tex, level, GL_FALSE, layer, access, translateFormat(config.format));
	//glUniform1i(location, static_cast<GLint>(unit));
}

void RenderResourcesGL::setUniform(RenderResources::UniformType type, const char* name, const void* data)
{
	GLint id;
	GL_CALL(glGetIntegerv(GL_CURRENT_PROGRAM, &id));

	GLint loc = GL_CALL(glGetUniformLocation(id, name));

	switch (type)
	{
	case RenderResources::UniformType::MATRIX4x4:
		GL_CALL(glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data));
		break;
	case RenderResources::UniformType::VEC2:
		GL_CALL(glUniform2f(loc, ((GLfloat*)data)[0], ((GLfloat*)data)[1]));
		break;
	case RenderResources::UniformType::INT:
		GL_CALL(glUniform1i(loc, *(int*)data));
		break;
	case RenderResources::UniformType::IVEC2:
		GL_CALL(glUniform2iv(loc, 1, (int*)data));
		break;
	default:
		throw std::runtime_error("Sorry, this uniform is not supported.");
		break;
	}
}

RenderResourcesGL::~RenderResourcesGL()
{
	cleanupPipeline();
	cleanupScene();
}


void RenderResourcesGL::cleanupScene()
{

}

void RenderResourcesGL::cleanupPipeline()
{
	if (m_primitive_enabled_buffer) {
		GL_CALL(glDeleteBuffers(1, &m_primitive_enabled_buffer));
	}
	
	for (GLuint tex : m_textures)
	{
		GL_CALL(glDeleteTextures(1, &tex));
	}
	for (GLuint buf : m_buffers)
	{
		if (buf != 0) {
			GL_CALL(glDeleteBuffers(1, &buf));
		}
	}
	for (GLuint buf : m_renderbuffers)
	{
		GL_CALL(glDeleteRenderbuffers(1, &buf));
	}
	for (GLuint fb : m_framebuffers)
	{
		GL_CALL(glDeleteFramebuffers(1, &fb));
	}
	for (GLuint prog : m_programs)
	{
		GL_CALL(glDeleteProgram(prog));
	}
	for (GLuint shad : m_shaders)
	{
		GL_CALL(glDeleteShader(shad));
	}
}