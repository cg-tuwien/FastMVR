#pragma once

#include <glad/glad.h>
//#include <GL/glew.h>
#include <stdexcept>
#include <iostream>

class GLException : public std::exception
{
private:
	GLint error_code;
public:
	GLException(GLint error) : error_code(error) {}
	virtual const char * what() const noexcept
	{
		switch (error_code)
		{
		case (GL_INVALID_ENUM):
			return "Enumeration parameter is not a legal enumeration for that function";
		case (GL_INVALID_VALUE):
			return "Illegal parameter value for that function";
		case (GL_INVALID_OPERATION):
			return "This operation cannot be executed in the current state of OpenGL";
		case (GL_STACK_OVERFLOW):
			return "Stack overflow";
		case (GL_STACK_UNDERFLOW):
			return "Stack underflow";
		case (GL_OUT_OF_MEMORY):
			return "Out of memory";
		case (GL_INVALID_FRAMEBUFFER_OPERATION):
			return "Operation could not be performed in the current state of the frame buffer";
		default:
			return "Unknown error! Please check error code!";
		}
	}
};

#ifdef _DEBUG						
#define GL_CALL(A) ([&]						\
{											\
	struct error_guard						\
	{										\
		~error_guard() noexcept(false)		\
		{									\
			GLenum error = glGetError();	\
			if (error != GL_NO_ERROR)		\
			{								\
				GLException ex(error);		\
				std::cerr << ex.what();		\
				throw ex;					\
			}								\
		}									\
	} guard;								\
	return A;								\
}())
#else							
#define GL_CALL(A) (A)		
#endif

#ifdef _DEBUG						
#define LOG_AND_IGNORE_GL_ERROR(A) ([&]		\
{											\
	struct error_guard						\
	{										\
		~error_guard() noexcept(false)		\
		{									\
			GLenum error = glGetError();	\
			if (error != GL_NO_ERROR)		\
			{								\
				GLException ex(error);		\
				std::cerr << ex.what();		\
			}								\
		}									\
	} guard;								\
	return A;								\
}())
#else							
#define LOG_AND_IGNORE_GL_ERROR(A) (A)		
#endif

class ShaderException : public std::exception
{
private:
	std::string log;
public:
	ShaderException(std::string&& log) : log(std::move(log)) {}

	virtual const char * what() const noexcept
	{
		return log.c_str();
	}
};

class ProgramException : public std::exception
{
private:
	std::string log;
public:
	ProgramException(std::string&& log) : log(std::move(log)) {}

	virtual const char * what() const noexcept
	{
		return log.c_str();
	}
};


static void checkShader(const GLint shader, const char* shadersrc)
{
	GLint isCompiled;
	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
	if (isCompiled != GL_TRUE)
	{
		GLint log_length;
		GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length));
		// vvvv Error Message here vvvv
		auto buffer = std::unique_ptr<char[]>(new char[log_length]);
		// ^^^^ Error Message here ^^^^ 
		GL_CALL(glGetShaderInfoLog(shader, log_length, &log_length, buffer.get()));

		ShaderException ex(std::string(buffer.get(), log_length - 1));
		std::cerr << shadersrc << std::endl;
		std::cerr << ex.what() << std::endl;
		throw ex;
	}
}

static void checkProgram(const GLint shader)
{
	GLint isLinked;
	GL_CALL(glGetProgramiv(shader, GL_LINK_STATUS, &isLinked));
	if (isLinked != GL_TRUE)
	{
		GLint log_length;
		GL_CALL(glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &log_length));
		auto buffer = std::unique_ptr<char[]>(new char[log_length]);
		GL_CALL(glGetProgramInfoLog(shader, log_length, &log_length, buffer.get()));

		ShaderException ex(std::string(buffer.get(), log_length - 1));
		std::cerr << ex.what();
		throw ex;
	}
}
