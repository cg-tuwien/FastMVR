#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <string>

class ViewPosGenerator
{
public:
	ViewPosGenerator(bool enable);
	void NextViewPos();
	void LockBuffer();
	void WaitBuffer();
	void ResetCounter();
	GLuint ReadCounter();

	glm::mat4 MatrixForCurrentViewPos();
	void SaveCurrentViewPos(std::string configName);

	bool IsEnabled() const { return m_isEnabled; }
	
	GLuint m_bufferHandle;
	void* m_mappedBufferData;

	GLsync m_fence;
	
	glm::vec3 m_curPos;
	float m_curRotX;
	float m_curRotY;

	std::string m_savedViewPosConfigs;

	bool m_isEnabled;
};

