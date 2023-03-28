#include <cgltf.h>
//Usess cgltf to load .gltf files




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



//Uses fucnctions to load them. Stores local mesh files 
class MeshLoader
{




};

