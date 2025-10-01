/*
 * Model.cpp
 *
 *  Created on: Sep 30, 2025
 *      Author: volokin
 */

#include "include/Model.h"

Model::Model(std::string const& path, bool gamma) : gammaCorrection(gamma) {
	loadModel(path);
}

void Model::Draw(Shader& shader){
	for(unsigned int i=0; i<meshes.size(); i++){
		meshes[i].Draw(shader);
	}
}

void Model::loadModel(std::string const& path){
	//reading file via assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if(!scene){
 		std::cout << "Error : Assimp : " << importer.GetErrorString() << std::endl;
		return;
	}
	
	//retrieve dir path of the filepath
	directory = path.substr(0, path.find_last_of('/'));
	
	//process assimp's root node recursively
	processNode(scene->mRootNode, scene);
}
	
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene){
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	
	//each of the mesh's vertices is to be walked through
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		Vertex vertex;
		glm::vec3 vector;
		
		//positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		
		//normals
		if(mesh->HasNormals()){
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
		}

		//texture coordinates
		if(mesh->mTextureCoords[0]){
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
 		
			//tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;

			//bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}
	
	//walk through each of the mesh's faces and retrieve the corresponding vertex indices
	for(unsigned int i=0; i<mesh->mNumFaces; i++){
		aiFace face = mesh->mFaces[i];
		//retrieve indices of the face and store in the indices vector
		for(unsigned int j=0; j<face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}	

	//process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	//the convention for sampler names is as follows: e.g "texture_diffuseN" where N is a sequential number ranging 1-MAX_SAMPLER_NUMBER

	//diffuse maps texture_diffuseN
	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	//specular maps
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	//normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	//height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	return Mesh(vertices, indices, textures);
}	

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma){
	std::string fileName = std::string(path);
	fileName = directory + '/' + fileName;
	
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);
	if(data){
		GLenum format;
		switch (nrComponents) {
			case 1:
				format = GL_RED;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
			default:
				break;
		}	
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}








