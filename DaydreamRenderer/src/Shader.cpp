#include "Shader.h"

Shader::Shader(const std::string & filepath)
{
	ShaderProgramSource source = ParseShaders(filepath);
	m_RendererID = CreateShaderProgram(source);
}

Shader::~Shader()
{
	glDeleteProgram(m_RendererID);
}

ShaderProgramSource Shader::ParseShaders(const std::string & filepath)
{
	// get the correct file names
	std::string vertexFilepath = filepath + "Vertex.shader";
	std::string geometryFilepath = filepath + "Geometry.shader";
	std::string fragmentFilepath = filepath + "Fragment.shader";
	bool hasGeometryShader = false;

	// deal with vertex shader first
	std::ifstream vertexStream(vertexFilepath);
	std::string vertexShaderCode;
	if (vertexStream.fail()) {
		std::cout << "Fail to load " << vertexFilepath << std::endl;
	}
	else {
		std::string vertexLine;
		while (getline(vertexStream, vertexLine)) {
			vertexShaderCode.append(vertexLine);
			vertexShaderCode.append("\n");
		}
	}

	// deal with fragment shader
	std::ifstream fragmentStream(fragmentFilepath);
	std::string fragmentShaderCode;
	if (fragmentStream.fail()) {
		std::cout << "Fail to load " << fragmentFilepath << std::endl;
	}
	else {
		std::string fragmentLine;
		while (getline(fragmentStream, fragmentLine)) {
			fragmentShaderCode.append(fragmentLine);
			fragmentShaderCode.append("\n");
		}
	}

	// is there a geometry shader?
	std::ifstream geometryStream(geometryFilepath);
	std::string geometryShaderCode;
	if (geometryStream.fail()) {
		// there is no geometry shader
		geometryShaderCode.append("There is no geometry shader:(");
		//std::cout << "No geometry shader:(" << std::endl;
	}
	else {
		hasGeometryShader = true;
		std::string geometryLine;
		while (getline(geometryStream, geometryLine)) {
			geometryShaderCode.append(geometryLine);
			geometryShaderCode.append("\n");
		}
	}

	return { vertexShaderCode, geometryShaderCode, fragmentShaderCode, hasGeometryShader };
}

unsigned int Shader::CompileShader(unsigned int shaderType, const std::string & shaderSource)
{
	unsigned int shader = glCreateShader(shaderType);
	const char* src = shaderSource.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	// TODO: Error handling
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(shader, length, &length, message);
		std::cout << "Failed to compile shader" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

unsigned int Shader::CreateShaderProgram(const ShaderProgramSource & source)
{
	unsigned int shaderProgram = glCreateProgram();
	unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, source.VertexSource);
	unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, source.FragmentSource);
	if (source.hasGeometryShader) {
		unsigned int geometryShader = CompileShader(GL_GEOMETRY_SHADER, source.GeometrySource);
		glAttachShader(shaderProgram, geometryShader);
	}

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void Shader::Bind()
{
	glUseProgram(m_RendererID);
}

void Shader::UnBind()
{
	glUseProgram(0);
}

unsigned int Shader::getID()
{
	return m_RendererID;
}

void Shader::SetUniformMatrix4fv(const GLchar* uniformName, glm::mat4 matrix)
{
	unsigned int location = glGetUniformLocation(m_RendererID, uniformName);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetUniformVector2fv(const GLchar * uniformName, glm::vec2 vector)
{
	unsigned int location = glGetUniformLocation(m_RendererID, uniformName);
	glUniform2fv(location, 1, glm::value_ptr(vector));
}

void Shader::SetUniformVector3fv(const GLchar * uniformName, glm::vec3 vector)
{
	unsigned int location = glGetUniformLocation(m_RendererID, uniformName);
	glUniform3fv(location, 1, glm::value_ptr(vector));
}

void Shader::SetUniform1f(const GLchar * uniformName, float v)
{
	unsigned int location = glGetUniformLocation(m_RendererID, uniformName);
	glUniform1f(location, v);
}

void Shader::SetUniform1i(const GLchar * uniformName, int v)
{
	unsigned int location = glGetUniformLocation(m_RendererID, uniformName);
	glUniform1i(location, v);
}
