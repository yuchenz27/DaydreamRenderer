#pragma once
#include <GL/glew.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

struct AABB {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
};

class Model {
public:
	Model(const std::string path);
	void Draw(Shader &shader);
	std::vector<Mesh> GetMeshes();
	glm::vec3 GetModelCenter();
	float GetModelFeet();
	float GetModelHead();
	AABB GetAABB();
	// give every value a very big number
	static void InitializeAABB(AABB &aabb);
	static AABB TransformAABB(AABB aabb, glm::mat4 model);
	static void LogAABB(const AABB aabb);
	void LogTextureInfo();
	unsigned int GetNumVertices();

private:
	std::vector<Mesh> meshes;
	// we retain this directory since we need to load textures from this same directory
	std::string m_Directory;
	std::vector<Texture> textures_loaded;
	// the order is xmin, xmax, ymin, ymax, zmin and zmax
	AABB m_AABB;
	unsigned int m_NumVertices;

	void loadModel(const std::string path);
	// recursive process
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene * scene);
	unsigned int static TextureFromFile(const char *path, const string &directory);
	unsigned int static TextureFromMemory(const char *path, const aiScene * scene);
};