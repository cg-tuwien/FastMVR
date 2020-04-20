#pragma once
#include <string>
#include <glad/glad.h>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <optional>
#include "RenderResources.h"
#include "math_utils.h"
#define GL_SCENE
#include "Scene.h"
#undef GL_SCENE

/**	Data for rendering which is returned by GeometryProviderBase and derived classes.
 *	Contains GPU objects that can be directly used for draw calls.
 */
struct GeometryRenderInfo
{
	/** GL-handle to the vertex buffer */
	GLuint m_vertex_buffer;
	/** GL-handle to the index buffer */
	GLuint m_index_buffer;
	/** GL-handle to the vertex array object */
	GLuint m_vao;

	/** Contains the config for a DrawElements-drawcall. */
	RenderResources::DrawElementsIndirectCommand m_drawcall_data;
	
	/** OPTIONAL GL-handle to a Uniform Buffer Object which contains a mapping to primitive-ids
	 *	from the ORIGINAL geometry. If this buffer is not set, it means that the primitive-ids in
	 *	the index buffer match the original geometry's primitive-ids 1:1.
	 */
	GLuint m_primitive_id_mapping_buffer;

	/** Where, in the buffer with the handle m_m_primitive_id_mapping_buffer, does the mapping start? */
	GLuint m_offset_into_primitive_id_mapping_buffer;
};



/** Abstract base class for "Geometry Providers".
 *
 *	A Geometry Provider's purpose is to provide geometry for 0..n draw calls for a
 *	specific view.
 */
class GeometryProviderBase
{
public:
	virtual ~GeometryProviderBase() {};
	
	virtual const std::string& name() { return m_name; }

	virtual void initWithBakedData(const std::vector<SceneVertex>& vertices,const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom) = 0;
	virtual bool requiresPrimitiveIdMapping() = 0;
	virtual size_t maxNumberOfPrimitiveMappingOffsets() { return 0; }
	virtual std::vector<GeometryRenderInfo> getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices) = 0;

	virtual size_t getDebugIndex()
	{
		return m_debug_index;
	}

	virtual bool getDebugModelEnabled()
	{
		return m_debug_mode_enabled;
	}

	virtual void setDebugModeEnabled(bool enable)
	{
		m_debug_mode_enabled = enable;
	}

	virtual void setDebugIndex(int index)
	{
		m_debug_index = index;
	}

	virtual void setDebugRenderEverything(bool doIt)
	{
		m_debug_mode_render_everything = doIt;
	}

	virtual bool getDebugRenderEverything()
	{
		return m_debug_mode_render_everything;
	}

protected:
	GeometryProviderBase(std::string_view name) : m_name{name}
	{}

	bool m_debug_mode_enabled = false;
	bool m_debug_mode_render_everything = false;
	size_t m_debug_index = 0;
	
private:
	std::string m_name;
};
