#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string GeometrySource;
	std::string FragmentSource;
	bool hasGeometryShader;
};

class Shader
{
private:
	unsigned int m_RendererID;
public:
	Shader(const std::string& filepath);
	~Shader();
	void Bind();
	void UnBind();
	unsigned int getID();
	void SetUniformMatrix4fv(const GLchar* uniformName, glm::mat4 matrix);
	void SetUniformVector2fv(const GLchar* uniformName, glm::vec2 vector);
	void SetUniformVector3fv(const GLchar* uniformName, glm::vec3 vector);
	void SetUniform1f(const GLchar* uniformName, float v);
	void SetUniform1i(const GLchar* uniformName, int v);

private:
	ShaderProgramSource ParseShaders(const std::string& filepath);
	unsigned int CompileShader(unsigned int shaderType, const std::string& shaderSource);
	unsigned int CreateShaderProgram(const ShaderProgramSource & source);

};
