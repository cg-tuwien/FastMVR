#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(std::string shader_code, std::string additionalShaderCodeLikeDefinesOrSo)
	: m_shader_code{ insert_into_shader_code_after_version(std::move(shader_code), std::move(additionalShaderCodeLikeDefinesOrSo)) }
{
}

void ComputePipeline::init(RenderResources* res)
{
	save_resources_ptr(res);
}

void ComputePipeline::buildProgram()
{
	m_program = resources()->buildProgram(m_shader_code);
	resources()->use(program_handle());
	resources()->getWorkgroupSizes(program_handle(), &m_work_group_sizes[0]);
	GL_CALL(glUseProgram(0));
	m_built = true;

}

void ComputePipeline::compute()
{
	assert(m_built);
	assert(work_group_size_x() == 1);
	assert(work_group_size_y() == 1);
	assert(work_group_size_z() == 1);

	resources()->use(program_handle());
	bindResources();
	GL_CALL(glDispatchCompute(1, 1, 1));
}

void ComputePipeline::compute(GLsizei width)
{
	assert(m_built);
	assert(work_group_size_y() == 1);
	assert(work_group_size_z() == 1);
	const auto num_groups_x = (width + work_group_size_x() - 1) / work_group_size_x();

	resources()->use(program_handle());
	bindResources();
	GL_CALL(glDispatchCompute(num_groups_x, 1, 1));
}

void ComputePipeline::compute(GLsizei width, GLsizei height)
{
	assert(m_built);
	assert(work_group_size_z() == 1);
	const auto num_groups_x = (width + work_group_size_x() - 1) / work_group_size_x();
	const auto num_groups_y = (height + work_group_size_y() - 1) / work_group_size_y();

	resources()->use(program_handle());
	bindResources();
	GL_CALL(glDispatchCompute(num_groups_x, num_groups_y, 1));
}

void ComputePipeline::compute(GLsizei width, GLsizei height, GLsizei depth)
{
	assert(m_built);
	const auto num_groups_x = (width  + work_group_size_x() - 1) / work_group_size_x();
	const auto num_groups_y = (height + work_group_size_y() - 1) / work_group_size_y();
	const auto num_groups_z = (depth  + work_group_size_z() - 1) / work_group_size_z();

	resources()->use(program_handle());
	bindResources();
	GL_CALL(glDispatchCompute(num_groups_x, num_groups_y, num_groups_z));
}
