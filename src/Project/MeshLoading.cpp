#pragma once
#include "include/MeshLoading.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

void MeshLoader::ParseAccessorPosition(cgltf_accessor const& accessor, Mesh& mesh)
{

}


void MeshLoader::ParseAccessorIndices(cgltf_accessor const& accessor, Mesh& mesh)
{

}



void MeshLoader::ParseAttribute(cgltf_attribute const& attribute, Mesh& mesh)
{
	if (attribute.type == cgltf_attribute_type::cgltf_attribute_type_position)
	{
		ParseAccessorPosition(*attribute.data, mesh);
	}

}

void MeshLoader::ParseMesh(cgltf_mesh const& mesh_cgltf, Mesh& mesh)
{
	for (cgltf_size i = 0; i < mesh_cgltf.primitives_count; ++i)
	{
		for (cgltf_size j = 0; j < mesh_cgltf.primitives[i].attributes_count; ++j)
		{
			ParseAttribute(mesh_cgltf.primitives[i].attributes[j], mesh);
		}

		ParseAccessorIndices(*mesh_cgltf.primitives[i].indices, mesh);
	}
}


//Usess cgltf to load .gltf files
void MeshLoader::loadGLTF(const char* filePath)
{
	//To Do: Add proper subnode support
	cgltf_options options{};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filePath, &data);

	if (result != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}

	//Load every buffer's data early
	if (cgltf_load_buffers(&options, data, filePath) != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}

	//Hardcoded 


	if (data != NULL)
		cgltf_free(data);
}




Mesh MeshLoader::loadDeccers(const char* filePath)
{
	cgltf_options options{};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filePath, &data);

	if (result != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}

	//Load every buffer's data early
	if (cgltf_load_buffers(&options, data, filePath) != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}

	//Iterate through the nodes to store the node and mesh data
	node_mesh_list.resize(data->nodes_count);



	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		if (data->nodes[i].mesh == nullptr)
			continue;

		node_mesh_list[i].hasMesh = true;

		cgltf_node* node_ptr = &data->nodes[i];
		if (node_ptr->has_scale)
			node_mesh_list[i].local_scale = glm::vec3(node_ptr->scale[0], node_ptr->scale[1], node_ptr->scale[2]);

		if (node_ptr->has_translation)
			node_mesh_list[i].local_translate = glm::vec3(node_ptr->translation[0], node_ptr->translation[1], node_ptr->translation[2]);


		//procoess this node's mesh stuff I guess

		cgltf_accessor* accessorPos = nullptr;
		cgltf_accessor* accessorIndices = nullptr;

		accessorIndices = node_ptr->mesh->primitives[0].indices;

		for (size_t i = 0; i < node_ptr->mesh->primitives[0].attributes_count; ++i)
		{
			if (accessorPos == nullptr && node_ptr->mesh->primitives[0].attributes[i].type == cgltf_attribute_type_position)
				accessorPos = node_ptr->mesh->primitives[0].attributes[i].data;
		}

		node_mesh_list[i].positions.resize(accessorPos->count);

		//cgltf_float p = 0.0f;
		for (size_t j = 0; j < accessorPos->count; ++j)
		{
			cgltf_accessor_read_float(accessorPos, j, glm::value_ptr(node_mesh_list[i].positions[j]), 3);

			//Apply the local transformations at access (TRS except no R)

			node_mesh_list[i].positions[j].x *= node_mesh_list[i].local_scale.x;
			node_mesh_list[i].positions[j].y *= node_mesh_list[i].local_scale.y;
			node_mesh_list[i].positions[j].z *= node_mesh_list[i].local_scale.z;


			node_mesh_list[i].positions[j].x += node_mesh_list[i].local_translate.x;
			node_mesh_list[i].positions[j].y += node_mesh_list[i].local_translate.y;
			node_mesh_list[i].positions[j].z += node_mesh_list[i].local_translate.z;
		}

		//std::vector<uint32_t> indicesTest;
		node_mesh_list[i].indices.resize(accessorIndices->count);


		//Because cgltf stores the pointer but loses the index, have to instead treat every submesh as separate and stich them
		//together later. Need to think of a better way to handle this for non Deccers though...
		//uint32_t offsetIndices = 0;
		//cgltf_accessor_read_uint(accessorIndices, 0, &offsetIndices, 1);

		uint32_t smallest_index = 1;

		for (size_t j = 0; j < accessorIndices->count; ++j)
		{
			//To Do: this function supports reading up to four at a time. Can ensure the stride is four in future
			cgltf_accessor_read_uint(accessorIndices, j, &node_mesh_list[i].indices[j], 1);


			//To Do: a way to offset these indices to zero per mesh in a more effiicent way than looping through them again?
			if (smallest_index)
				smallest_index = node_mesh_list[i].indices[j] < smallest_index ? node_mesh_list[i].indices[j] : smallest_index;
		}


		//Not needed for Deccers but needed if a gltf file ends up using a different bufferView for their indices (which is most other files)

		//Offset the indices back to zero for each mesh (for combination later). 
		//We take the smallest index which is also the difference of the all the indices from zero and offset it so
		//that the index list 'starts' at zero
		if (smallest_index)
		{
			std::for_each(node_mesh_list[i].indices.begin(), node_mesh_list[i].indices.end(), [&](uint32_t& index)
				{
					index -= smallest_index;
				});
		}


	}


	//If you want to render as the same mesh we need to offset the indices accordingly (again but in our own order)
	//To Do: Account for child/parent relationships so that the transformations are hirerachial (right now its only local)



	Mesh combinedMesh;

	//std::vector<glm::vec3> combinedPositions;
	//std::vector<uint32_t> combinedIndices;



	uint32_t index_offset = 0;
	for (node_with_mesh& mesh : node_mesh_list)
	{
		uint32_t biggest_number = 0;

		//Offset all indices relative to the previous meshes and also update the offset for next iteration
		std::for_each(mesh.indices.begin(), mesh.indices.end(), [&](uint32_t& index) {
			index += index_offset;
			biggest_number = biggest_number < index ? index : biggest_number; });

		combinedMesh.positions.insert(combinedMesh.positions.end(), mesh.positions.begin(), mesh.positions.end());
		combinedMesh.indices.insert(combinedMesh.indices.end(), mesh.indices.begin(), mesh.indices.end());

		index_offset = biggest_number + 1;
	}

	return combinedMesh;
}




void MeshLoadingTest::loadGLTF_Basic(const char* filePath, std::vector<glm::vec3>& positions,
	std::vector<uint32_t>& indices)
{
	//We try a basic possible file that I created. A plane with four vertices. We only carea bout the position

	//To Do: Add proper subnode support
	cgltf_options options{};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filePath, &data);

	if (result != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}

	//Load every buffer's data early
	if (cgltf_load_buffers(&options, data, filePath) != cgltf_result_success)
	{
		//To Do: Some kinda warning/error message
	}


	//To Do: Allow a way to parse this mesh by mesh

	cgltf_accessor* accessorPos = nullptr;
	cgltf_accessor* accessorIndices = nullptr;

	accessorIndices = data->meshes[0].primitives[0].indices;

	for (size_t i = 0; i < data->meshes[0].primitives[0].attributes_count; ++i)
	{
		if (accessorPos == nullptr && data->meshes[0].primitives[0].attributes[i].type == cgltf_attribute_type_position)
			accessorPos = data->meshes[0].primitives[0].attributes[i].data;
	}


	//cgltf_accessor const& accessorPos = data->accessors[0];
	//cgltf_accessor const& accessorIndices = data->accessors[3];
	
	//I already know the first buffer is position just hardcode that stuff
	// https://github.com/jkuhlmann/cgltf/blob/master/test/test_conversion.cpp
	// 
	// 
	// 
	//accessor.buffer_view[0].buffer



	//std::vector<std::array<cgltf_float, 3>> positionsTest;
	//std::vector<glm::vec3> positionsTest;
	positions.resize(accessorPos->count);

	//cgltf_float p = 0.0f;
	for (size_t i = 0; i < accessorPos->count; ++i)
	{
		cgltf_accessor_read_float(accessorPos, i, glm::value_ptr(positions[i]), 3);
	}

	//std::vector<uint32_t> indicesTest;
	indices.resize(accessorIndices->count);

	for (size_t i = 0; i < accessorIndices->count; ++i)
	{
		cgltf_accessor_read_uint(accessorIndices, i, indices.data() + i, 1);
	}

	if (data != NULL)
		cgltf_free(data);

}

