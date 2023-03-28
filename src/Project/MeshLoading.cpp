#pragma once
#include "include/MeshLoading.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>


//Usess cgltf to load .gltf files
Mesh MeshLoader::loadGLTF(const char* filePath)
{
	//To Do: Add proper subnode support


	cgltf_options options{};
	
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, "scene.gltf", &data);
	
	

	if (result == cgltf_result_success)
	{
		/* TODO make awesome stuff */
		cgltf_free(data);
	}

	return Mesh{};
}