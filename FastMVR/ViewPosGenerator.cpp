#include "ViewPosGenerator.h"
#include <random>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include "view_matrix_configs.h"

ViewPosGenerator::ViewPosGenerator(bool enable) : m_isEnabled{enable}
{
	// TODO:
	// - https://www.lighthouse3d.com/tutorials/opengl-atomic-counters/
	// - https://community.khronos.org/t/fastest-way-to-reset-an-atomic-counter/72804
	//

	// Creation:
	glGenBuffers(1, &m_bufferHandle);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_bufferHandle);
	const GLbitfield bufferFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	const GLsizeiptr bufferSize = sizeof(GLuint) * 1;
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, bufferSize, nullptr, bufferFlags);
	// Mapping (only once after creation...):
	m_mappedBufferData = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, bufferSize, bufferFlags);
	// unbind the buffer 
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 5, m_bufferHandle); // layout (binding = 5, offset = 0)
}

void ViewPosGenerator::LockBuffer()
{
  if( m_fence )
  {
	glDeleteSync( m_fence );	
  }
  m_fence = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
}

void ViewPosGenerator::WaitBuffer()
{
  if( m_fence )
  {
    while( true )	
	{
	  GLenum waitReturn = glClientWaitSync( m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1 );
	  if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
	    return;
    }
  }
}

void ViewPosGenerator::ResetCounter()
{
	GLuint initValue = 0;
	*reinterpret_cast<GLuint*>(m_mappedBufferData) = initValue;
	const GLsizeiptr bufferSize = sizeof(GLuint) * 1;
}

GLuint ViewPosGenerator::ReadCounter()
{
	return *reinterpret_cast<GLuint*>(m_mappedBufferData);
}

void ViewPosGenerator::NextViewPos()
{
	// Ausdehnungen robot_lab_unity:
	// x: -10.61 .. 10.49
	// y:   1.56 ..  5.38
	// z: -18.61 .. 23.90
	// 
	// Ausdehnungen viking_village_unity:
	// x: -125.57 ..  63.69
	// y:    6.41 ..  30.62
	// z:  -31.52 ..  62.71
	//
	// Ausdehnungen sponza:
	// x: -12.43 .. 12.33
	// y:   1.62 ..  7.92
	// z:  -4.99 ..  5.05
	//
	// Ausdehnungen bistro:
	// x: -16.75 ..  24.85
	// y:   1.91 ..  23.47
	// z: -35.92 ..  -3.23
	// und
	// x:  -3.95 ..   6.31
	// y:   1.62 ..   7.54
	// z: -16.58 ..  14.34
	// und
	// x:   4.68 ..  59.04
	// y:   1.82 ..  11.00
	// z:  14.20 ..  21.68
	//
	// Ausdehnungen gallery:
	// x:  -3.85 ..  3.80
	// y:   1.03 ..  5.16
	// z: -12.74 .. 10.73
	//
	// Ausdehnungen san miguel:
	// x:   6.50 .. 26.01
	// y:   1.06 .. 10.77
	// z:  -2.90 .. 12.67

	static std::default_random_engine generator;
	//static std::uniform_real_distribution<float> dist_pos_x(-10.61f,  10.49f);
	//static std::uniform_real_distribution<float> dist_pos_y(  1.56f,   5.38f);
	//static std::uniform_real_distribution<float> dist_pos_z(-18.61f,  23.90f);
	//static std::uniform_real_distribution<float> dist_pos_x(-125.57f,  63.69f);
	//static std::uniform_real_distribution<float> dist_pos_y(   6.41f,  30.62f);
	//static std::uniform_real_distribution<float> dist_pos_z( -31.52f,  62.71f);
	static std::uniform_real_distribution<float> dist_pos_x(   4.68, 59.04);
	static std::uniform_real_distribution<float> dist_pos_y(   1.82, 11.00);
	static std::uniform_real_distribution<float> dist_pos_z(  14.20, 21.68);
	static std::uniform_real_distribution<float> dist_rot_horiz(glm::radians(-180.0f), glm::radians(180.0f));
	static std::uniform_real_distribution<float> dist_rot_vert(glm::radians(-70.0f), glm::radians(70.0f));
	
	m_curPos = glm::vec3(dist_pos_x(generator), dist_pos_y(generator), dist_pos_z(generator));
	m_curRotX = dist_rot_vert(generator);
	m_curRotY = dist_rot_horiz(generator);
}

glm::mat4 ViewPosGenerator::MatrixForCurrentViewPos()
{
	auto M = calc_view_matrix_with_pos_and_transforms(glm::vec3(0.f, 0.f, 0.f), glm::mat4(1.0f), glm::translate(glm::mat4(1.0f), m_curPos) * glm::rotate(glm::mat4(1.0f), m_curRotY, glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(1.0f), m_curRotX, glm::vec3(1.f, 0.f, 0.f)));
	return M;
}

void ViewPosGenerator::SaveCurrentViewPos(std::string configName)
{
	m_savedViewPosConfigs += "add_view_matrix_config(\"" + configName + "_1x1_close\", 1,1, 0.1f, 0.1f, "
		+ fmt::format("glm::mat4(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3({}f, {}f, {}f)) * glm::rotate(glm::mat4(1.0f), {}f, glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(1.0f), {}f, glm::vec3(1.f, 0.f, 0.f)));\n",
																		     m_curPos.x, m_curPos.y, m_curPos.z,				m_curRotY,													m_curRotX);
	static int viewCounter = 0;
	viewCounter++;
}