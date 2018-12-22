#include "Model.h"

Model::Model(const std::string path) {
	InitializeAABB(m_AABB);
	loadModel(path);
}

void Model::Draw(Shader &shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

std::vector<Mesh> Model::GetMeshes()
{
	return meshes;
}

glm::vec3 Model::GetModelCenter() {
	glm::vec3 center;
	center.x = (m_AABB.xmin + m_AABB.xmax) / 2.0f;
	center.y = (m_AABB.ymin + m_AABB.ymax) / 2.0f;
	center.z = (m_AABB.zmin + m_AABB.zmax) / 2.0f;
	return center;
}

float Model::GetModelFeet() {
	return m_AABB.ymin;
}

float Model::GetModelHead() {
	return m_AABB.ymax;
}


void Model::InitializeAABB(AABB &aabb)
{
	aabb.xmin = 10000.0f;
	aabb.xmax = -10000.0f;
	aabb.ymin = 10000.0f;
	aabb.ymax = -10000.0f;
	aabb.zmin = 10000.0f;
	aabb.zmax = -10000.0f;
}

AABB Model::TransformAABB(AABB inputAABB, glm::mat4 tMatrix)
{
	// original vertices
	glm::vec3 vertices[8];
	vertices[0] = glm::vec3(inputAABB.xmin, inputAABB.ymin, inputAABB.zmin);
	vertices[1] = glm::vec3(inputAABB.xmin, inputAABB.ymin, inputAABB.zmax);
	vertices[2] = glm::vec3(inputAABB.xmin, inputAABB.ymax, inputAABB.zmin);
	vertices[3] = glm::vec3(inputAABB.xmin, inputAABB.ymax, inputAABB.zmax);
	vertices[4] = glm::vec3(inputAABB.xmax, inputAABB.ymin, inputAABB.zmin);
	vertices[5] = glm::vec3(inputAABB.xmax, inputAABB.ymin, inputAABB.zmax);
	vertices[6] = glm::vec3(inputAABB.xmax, inputAABB.ymax, inputAABB.zmin);
	vertices[7] = glm::vec3(inputAABB.xmax, inputAABB.ymax, inputAABB.zmax);
	// transform into the new space
	vertices[0] = (tMatrix * glm::vec4(vertices[0], 1.0f)).xyz;
	vertices[1] = (tMatrix * glm::vec4(vertices[1], 1.0f)).xyz;
	vertices[2] = (tMatrix * glm::vec4(vertices[2], 1.0f)).xyz;
	vertices[3] = (tMatrix * glm::vec4(vertices[3], 1.0f)).xyz;
	vertices[4] = (tMatrix * glm::vec4(vertices[4], 1.0f)).xyz;
	vertices[5] = (tMatrix * glm::vec4(vertices[5], 1.0f)).xyz;
	vertices[6] = (tMatrix * glm::vec4(vertices[6], 1.0f)).xyz;
	vertices[7] = (tMatrix * glm::vec4(vertices[7], 1.0f)).xyz;
	// compute the new AABB
	AABB resultAABB;
	InitializeAABB(resultAABB);
	for (int i = 0; i < 8; i++) {
		if (resultAABB.xmin > vertices[i].x)
			resultAABB.xmin = vertices[i].x;
		if (resultAABB.xmax < vertices[i].x)
			resultAABB.xmax = vertices[i].x;
		if (resultAABB.ymin > vertices[i].y)
			resultAABB.ymin = vertices[i].y;
		if (resultAABB.ymax < vertices[i].y)
			resultAABB.ymax = vertices[i].y;
		if (resultAABB.zmin > vertices[i].z)
			resultAABB.zmin = vertices[i].z;
		if (resultAABB.zmax < vertices[i].z)
			resultAABB.zmax = vertices[i].z;
	}
	return resultAABB;
}

void Model::LogAABB(const AABB aabb)
{
	std::cout << "The AABB is:" << std::endl;
	std::cout << "xmin: " << aabb.xmin << std::endl;
	std::cout << "xmax: " << aabb.xmax << std::endl;
	std::cout << "ymin: " << aabb.ymin << std::endl;
	std::cout << "ymax: " << aabb.ymax << std::endl;
	std::cout << "zmin: " << aabb.zmin << std::endl;
	std::cout << "zmax: " << aabb.zmax << std::endl;
}

void Model::LogTextureInfo() {
	for (int i = 0; i < meshes.size(); i++) {
		std::cout << "Mesh " << (i + 1) << ":" << std::endl;
		meshes[i].LogTextureInfo();
		std::cout << std::endl;
	}
}

unsigned int Model::GetNumVertices() {
	return m_NumVertices;
}

AABB Model::GetAABB()
{
	return m_AABB;;
}

void Model::loadModel(const std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	m_Directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode * node, const aiScene * scene)
{
	// process all current meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_NumVertices += mesh->mNumVertices;
		meshes.push_back(processMesh(mesh, scene));
	}

	// then go to all children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	// process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vector2;
			vector2.x = mesh->mTextureCoords[0][i].x;
			vector2.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoord = vector2;
		}
		else {
			vertex.TexCoord = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);

		// compute the AABB
		// notice that this AABB is in model space
		if (vertex.Position.x < m_AABB.xmin)
			m_AABB.xmin = vertex.Position.x;
		if (vertex.Position.x > m_AABB.xmax)
			m_AABB.xmax = vertex.Position.x;
		if (vertex.Position.y < m_AABB.ymin)
			m_AABB.ymin = vertex.Position.y;
		if (vertex.Position.y > m_AABB.ymax)
			m_AABB.ymax = vertex.Position.y; 
		if (vertex.Position.z < m_AABB.zmin)
			m_AABB.zmin = vertex.Position.z;
		if (vertex.Position.z > m_AABB.zmax)
			m_AABB.zmax = vertex.Position.z;
	}

	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// process textures
	// each mesh can only have one single material, while a material can have multiple diffuse and specular textures
	if (mesh->mMaterialIndex >= 0) {
		// we get the material of the current mesh
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// TODO: can I check here whether this material is using embedded textures?
		// all I want here is a vector of textures, so it doesn't matter whether they are embedded or not
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	
	return Mesh(vertices, indices, textures);
}

// we need the scene object because we need access to embedded textures
std::vector<Texture> Model::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName, const aiScene * scene)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// print out the path of the current texture file :)
		//std::cout << str.C_Str() << std::endl;
		// TODO: check whether this texture is embedded and design solutions to deal with embedded textures
		bool embedded = false;
		if (str.data[0] == '*') {
			embedded = true;
		}

		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			Texture texture;
			if (!embedded)
				texture.id = TextureFromFile(str.C_Str(), m_Directory);
			else
				texture.id = TextureFromMemory(str.C_Str(), scene);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

// for non-embedded textures
unsigned int Model::TextureFromFile(const char * path, const string & directory) {

	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// for embedded textures
unsigned int Model::TextureFromMemory(const char * path, const aiScene * scene) {

	// TODO: make these conversions more efficient
	std::string str(path);
	std::string embeddedIndex = str.substr(1, str.length());
	aiTexture * embeddedTexture = scene->mTextures[atoi(embeddedIndex.c_str())];

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *image_data;
	if (embeddedTexture->mHeight == 0) {
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth, &width, &height, &nrComponents, 0);
	} else {
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth * embeddedTexture->mHeight, &width, &height, &nrComponents, 0);
	}
	if (image_data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(image_data);
	} else {
		std::cout << "Embedded Texture failed to load at path: " << path << std::endl;
		stbi_image_free(image_data);
	}

	return textureID;
}
