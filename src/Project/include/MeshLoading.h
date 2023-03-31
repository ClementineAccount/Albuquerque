#pragma once

#include <string>
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

struct Mesh
{
	using indexType = uint32_t;
	std::vector<indexType> indices;

	//To Do: Convert this to AOS later on (std::vector<Vertex>)
	std::vector<glm::vec3> positions;

	std::vector<glm::vec4> colors;
};



// Uses functions to load meshes with cgltf. Stores local mesh files 
// Why a class? Can allow multi-threaded loading of GLTF files with multiple MeshLoaders in the future

class cgltf_mesh;
class cgltf_accessor;
class cgltf_buffer_view;
class cgltf_attribute;



//So I can test my understanding quicker iteration

class MeshLoadingTest
{
public:

	static void loadGLTF_Basic(const char* filePath, std::vector<glm::vec3>& pos,
		std::vector<uint32_t>& indices);
};



class MeshLoader
{


public:

	void loadGLTF(const char* filePath);

	//Testing deccers cube
	Mesh loadDeccers(const char* filePath);

private:

	void ParseMesh(cgltf_mesh const& mesh_cgltf, Mesh& mesh);
	void ParseAccessorPosition(cgltf_accessor const& accessor, Mesh& mesh);

	void ParseAttribute(cgltf_attribute const& attribute, Mesh& mesh);

	void ParseAccessorIndices(cgltf_accessor const& accessor, Mesh& mesh);
	
	
	//void ParseAttribute(cgltf_attribute const& attribute, Mesh& mesh);
	void ParseBufferViewPosition(cgltf_buffer_view const& buffer_view, std::vector<glm::vec3>& pos);




	//Not a mesh type just for parsing data

	//To Do: fix alignment of struct when refactoring
	struct node_with_mesh
	{
		bool hasMesh = false;

		//Apply this when traversing the node
		glm::mat4 parent_transform{ 1.0f };

		//Apply these transformations locally to the mesh it belongs to
		glm::vec3 local_scale{1.0f, 1.0f, 1.0f};
		glm::vec3 local_translate{0.0f, 0.0f, 0.0f};


		//based off the accessor's data. Used if want to combine the mesh to ensure no overlap of indicesTest
		uint32_t index_id = 0;

		//3D only need to add support for 2D if the accessor states it
		std::vector<glm::vec3> positions;
		std::vector<uint32_t> indices;

		glm::vec4 base_color_factor;

		std::vector<glm::vec4> colorList;
	};




	private:

		std::vector<node_with_mesh> node_mesh_list;


		//For the testing but we use a local version for the deccers (to do: remove it ig)
		std::vector<glm::vec3> positionsTest;
		std::vector<Mesh::indexType> indicesTest;


};

