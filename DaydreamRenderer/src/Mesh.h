#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
};

struct Texture {
	unsigned int id;
	// diffuse or specular?
	std::string type;
	// this is actually the file name of the texture not the file path, very perplexing
	std::string path;
};

class Mesh {
public:
	// mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	// methods
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader &shader);
	unsigned int GetVAO();
	// print out the precise description of all textures in this mesh
	void LogTextureInfo();

private:
	// rendering data
	unsigned int VAO, VBO, EBO;
	// initialize buffer objects
	void setupMesh();
};