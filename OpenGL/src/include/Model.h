/*
 * Model.h
 *
 *  Created on: Sep 30, 2025
 *      Author: volokin
 */

#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_


#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>



unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);

class Model {
public:
	std::vector<Texture> loadedTextures;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;
	
	Model(std::string const& path, bool gamma = false);
	void Draw(Shader& shader);
	
private:
	void loadModel(std::string const& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
};



#endif /* SRC_MODEL_H_ */
