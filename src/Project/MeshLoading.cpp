#pragma once
#include "include/MeshLoading.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>


#include <glad/glad.h>

void MeshLoader::ParseAccessorPosition(cgltf_accessor const& accessor, Mesh& mesh)
{
	//Testing if I can just create the VAO directly here

	uint32_t vbo = 0;
	//uint32_t vao = 0;

	glCreateBuffers(1, &vbo);

	//will only consider the positions
	glNamedBufferStorage(vbo, accessor.buffer_view->size, accessor.buffer_view->buffer, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &vao);

	size_t values_per_vertex = 3;
	values_per_vertex = accessor.type;


	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec3));
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribFormat(vao, 0, values_per_vertex, accessor.component_type, GL_FALSE, 0);

	glVertexArrayAttribBinding(vao, 0, 0);
}


void MeshLoader::ParseAccessorIndices(cgltf_accessor const& accessor, Mesh& mesh)
{
	uint32_t ibo = 0;

	glCreateBuffers(1, &ibo);

	//To Do: Reasonable max buffer chunk size
	unsigned char index_buffer_chunk[256]{};

	//Where the pointer starts in the buffer data
	char* start_ptr = static_cast<char*>(accessor.buffer_view->buffer->data);
	start_ptr = start_ptr + accessor.buffer_view->offset;
	std::memcpy(index_buffer_chunk, start_ptr, accessor.buffer_view->size);

	glNamedBufferStorage(ibo, sizeof(uint32_t) * accessor.count, index_buffer_chunk, GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vao, ibo);
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

	loadedMeshes.resize(data->meshes_count);


	cgltf_mesh* curr_mesh_ptr = NULL;
	cgltf_primitive* curr_prim_ptr = NULL;




	//Parse each mesh
	for (cgltf_size i = 0; i < data->meshes_count; ++i)
	{
		curr_mesh_ptr = data->meshes + i;
		//To Do: Add nullptr check

		if (curr_mesh_ptr != NULL)
			ParseMesh(*curr_mesh_ptr, loadedMeshes[i]);
	}

	if (data != NULL)
		cgltf_free(data);
}