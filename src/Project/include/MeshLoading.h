#pragma once

#include <string>
#include <vector>
#include <array>
#include <glm/vec3.hpp>


struct Mesh
{
	using indexType = uint32_t;
	std::vector<indexType> indices;

	//To Do: Convert this to AOS later on (std::vector<Vertex>)
	std::vector<glm::vec3> positions;
};



// Uses functions to load meshes with cgltf. Stores local mesh files 
// Why a class? Can allow multi-threaded loading of GLTF files with multiple MeshLoaders in the future

class cgltf_mesh;
class cgltf_accessor;
class cgltf_buffer_view;
class cgltf_attribute;

class MeshLoader
{




public:

	void loadGLTF(const char* filePath);

	//To Do: Allow a more elegant way to store submeshes and meshes relative to each other
	//as well as considering batching and instancing 
	std::vector<Mesh> loadedMeshes;



	//Testing if I can just create the VAO directly
	
	
	uint32_t vao;


private:

	void ParseMesh(cgltf_mesh const& mesh_cgltf, Mesh& mesh);
	void ParseAccessorPosition(cgltf_accessor const& accessor, Mesh& mesh);

	void ParseAttribute(cgltf_attribute const& attribute, Mesh& mesh);

	void ParseAccessorIndices(cgltf_accessor const& accessor, Mesh& mesh);
	
	
	//void ParseAttribute(cgltf_attribute const& attribute, Mesh& mesh);
	//void ParseBufferViewPosition(cgltf_buffer_view const& buffer_view, std::vector<glm::vec3>& pos);

	struct meta_mesh
	{


	private:





		std::vector<glm::vec3> positions;
		std::vector<Mesh::indexType> indices;
	};
};

