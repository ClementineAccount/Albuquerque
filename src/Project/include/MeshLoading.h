#pragma once

#include <string>
#include <vector>
#include <glm/vec3.hpp>


//To Do: Move this to its own files
struct Vertex
{
	glm::vec3 pos;

	//To Do: add normals, uvs and all that here
};


struct Mesh
{
	using indexType = uint32_t;

	std::vector<Vertex> vertices;
	std::vector<indexType> indices;

	std::string meshName = "Default\n";
};



//Uses functions to load meshes with cgltf. Stores local mesh files 
class MeshLoader
{
	static Mesh loadGLTF(const char* filePath);
};

