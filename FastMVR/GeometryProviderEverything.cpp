#include "GeometryProviderEverything.h"
#include "GLHelper.h"

GeometryProviderEverything::GeometryProviderEverything()
	: GeometryProviderBase("evth")
{}

void GeometryProviderEverything::initWithBakedData(const std::vector<SceneVertex>& vertices,const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom)
{
	// struct GeometryRenderData
	// {
	// 	GLuint m_vertex_buffer;
	// 	GLuint m_index_buffer;
	// 	GLuint m_vao;
	// 	std::optional<GLuint> m_primitive_ids_buffer;
	// };
	
	m_stored_indices = indices;
	
	auto numVertices = vertices.size();
	
	GL_CALL(glGenBuffers(1, &m_render_data.m_vertex_buffer));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_render_data.m_vertex_buffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(SceneVertex) * numVertices, vertices.data(), GL_STATIC_DRAW));
	
	GL_CALL(glCreateVertexArrays(1, &m_render_data.m_vao));
	GL_CALL(glBindVertexArray(m_render_data.m_vao));
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_POS, SceneVertex::SIZE_POS, SceneVertex::TYPE_POS, GL_FALSE, sizeof(SceneVertex), (void*)offsetof(SceneVertex, position)));
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_NORM, SceneVertex::SIZE_NORM, SceneVertex::TYPE_NORM, GL_FALSE, sizeof(SceneVertex), (void*)offsetof(SceneVertex, normal)));
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_TRIANGLEID, SceneVertex::SIZE_TRIANGLEID, SceneVertex::TYPE_TRIANGLEID, GL_FALSE, sizeof(SceneVertex), (void*)offsetof(SceneVertex, triangle_id)));
	
	auto numIndices = indices.size();

	GL_CALL(glGenBuffers(1, &m_render_data.m_index_buffer));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_render_data.m_index_buffer));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * numIndices, indices.data(), GL_STATIC_DRAW));

	// Gather the data for a draw call:
	// typedef  struct {
	//     GLuint  count;
	//     GLuint  instanceCount;
	//     GLuint  firstIndex;
	//     GLuint  baseVertex;
	//     GLuint  baseInstance;
	// } DrawElementsIndirectCommand;
	m_render_data.m_drawcall_data = {
		static_cast<GLuint>(numIndices),
		1u, // one instance by default
		sizeof(uint32_t) * 0, // Specifies a byte offset (cast to a pointer type) into the buffer bound to GL_ELEMENT_ARRAY_BUFFER to start reading indices from.
		0u, // Specifies a constant that should be added to each element of indices when chosing elements from the enabled vertex arrays.
		0u  // Specifies the base instance for use in fetching instanced vertex attributes.
	};
	
	// Primitive-IDs match exactly the original. Hence, no buffer containing a primitive-ids mapping
	m_render_data.m_primitive_id_mapping_buffer = 0;
}

GeometryProviderEverything::~GeometryProviderEverything()
{
	if (m_render_data.m_vao)
	{
		GL_CALL(glDeleteVertexArrays(1, &m_render_data.m_vao));
	}
	if (m_render_data.m_index_buffer)
	{
		GL_CALL(glDeleteBuffers(1, &m_render_data.m_index_buffer));
	}
	if (m_render_data.m_vertex_buffer)
	{
		GL_CALL(glDeleteBuffers(1, &m_render_data.m_vertex_buffer));
	}
}

std::vector<GeometryRenderInfo> GeometryProviderEverything::getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices)
{
	// Ignore the matrix, just provide everything always
	return { m_render_data };
}
