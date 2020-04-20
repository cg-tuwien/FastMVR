#pragma once

#include <glm/glm.hpp>

#ifdef GL_SCENE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <GL/GL.h>
#endif

struct SceneObject
{
	glm::uvec2 fromTo;
	std::vector<glm::mat4> matrices;
};

struct SceneVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	uint32_t triangle_id;

	static const uint32_t ID_POS = 0;
	static const uint32_t ID_NORM = 1;
	static const uint32_t ID_TRIANGLEID = 2;

	static const uint32_t SIZE_POS = 3;
	static const uint32_t SIZE_NORM = 3;
	static const uint32_t SIZE_TRIANGLEID = 1;

#ifdef GL_SCENE
	static const GLuint TYPE_POS = GL_FLOAT;
	static const GLuint TYPE_NORM = GL_FLOAT;
	static const GLuint TYPE_TRIANGLEID = GL_UNSIGNED_INT;
#endif
};