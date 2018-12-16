#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}

void Mesh::Draw(Shader &shader)
{
	// bind appropriate textures
	unsigned int diffuseNum = 1;
	unsigned int specularNum = 1;
	for (unsigned int i = 0; i < textures.size(); i++) {
		// active proper texture unit before binding
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNum++);
		else if (name == "texture_specular")
			number = std::to_string(specularNum++);
		// TODO: does this material prefix fit all shaders? 
		const std::string uniformLoc = "material." + name + number;
		const char* uniformLocPointer = uniformLoc.c_str();
		shader.SetUniform1i(uniformLocPointer, i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	// NOTICE: if there is no specular texture, the texture_specular1 uniform in the shader will be bound to the first diffuse map
	glActiveTexture(GL_TEXTURE0);

	// draw call
	glBindVertexArray(VAO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

unsigned int Mesh::GetVAO()
{
	return VAO;
}

void Mesh::LogTextureInfo() {
	//std::cout << "The total number of texture maps: " << textures.size() << std::endl;
	unsigned int diffuseNUM = 0;
	unsigned int specularNUM = 0;
	for (int i = 0; i < textures.size(); i++) {
		std::string textureType = textures[i].type;
		if (textureType == "texture_diffuse") {
			diffuseNUM++;
		} else if (textureType == "texture_specular") {
			specularNUM++;
		}
		// print out the name of the current texture
		std::cout << textures[i].path << std::endl;
	}
	std::cout << "The number of diffuse texture maps: " << diffuseNUM << std::endl;
	std::cout << "The number of specular texture maps:" << specularNUM << std::endl;
}

void Mesh::setupMesh()
{
	// generate buffer objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind buffers
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

	// unbind everything
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
