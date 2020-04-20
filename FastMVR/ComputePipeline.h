#pragma once
#include "RenderResources.h"
#include "RenderResourcesGL.h"
//#include <GL/glew.h>
#include <glad/glad.h>
#include <array>
#include "string_utils.h"
#include "GLHelper.h"

class ComputePipeline
{
public:
	ComputePipeline(std::string shader_code, std::string additionalShaderCodeLikeDefinesOrSo = "");
	virtual void init(RenderResources* resources);
	virtual void buildProgram();
	virtual void bindResources() {}
	virtual void compute();
	virtual void compute(GLsizei width);
	virtual void compute(GLsizei width, GLsizei height);
	virtual void compute(GLsizei width, GLsizei height, GLsizei depth);

	GLint work_group_size_x() const { return m_work_group_sizes[0]; }
	GLint work_group_size_y() const { return m_work_group_sizes[1]; }
	GLint work_group_size_z() const { return m_work_group_sizes[2]; }

	auto program_handle() const { return m_program; }
	
	inline void save_resources_ptr(RenderResources* ptr) 
	{ 
		assert(nullptr != dynamic_cast<RenderResourcesGL*>(ptr)); 
		m_resources = ptr; 
	}

	inline RenderResourcesGL* resources() const 
	{ 
		return static_cast<RenderResourcesGL*>(m_resources); 
	}

protected:
	std::string m_shader_code;

	RenderResources* m_resources;

	int m_program;
	std::array<GLint, 3> m_work_group_sizes;
	bool m_built = false;
	
};

