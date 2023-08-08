#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
#include <array>
#include <optional>

#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>


namespace VoxelStuff
{
    struct Transform
    {
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    };


    struct ObjectUniform
    {
        glm::mat4 modelTransform;
    };


    //So this is like one voxel object. Right now it naively uses one draw call per voxel
    //which I predict will have tons of performance issues very quickly, but we gonna avoid premature optimization
    struct Voxel
    {
        Voxel(Transform transform = Transform()); //Imagine making a default constructor (cant be me)

        //bool isRendering;

        //Maybe I might as well build that modelTransform in place anyways?
        Transform transform;

        //Fwog example for 02_deferred
        ObjectUniform objectUniform;

    };

    struct Grid
    {
        Grid();

        // Lets try just an array of Voxels first
        // except I am going to make it a vector because of really good reasons:
        // (1): the big lazy

        //To Do: Do a 2D grid displacement
        static size_t constexpr numVoxelMax = 100;
        std::vector<Voxel> voxelGrid;

        //Fwog example for 02_deferred

        std::vector<ObjectUniform> objectUniforms;
        std::optional<Fwog::Buffer> objectBuffer;

        //non owning pointer to the mesh buffer
        Albuquerque::FwogHelpers::MeshBuffer* voxelMeshBufferRef;

        std::optional<Fwog::GraphicsPipeline> pipeline;

        void Update();

        void Draw(Fwog::Texture const& textureAlbedo, Fwog::Sampler const& sampler, ViewData const& viewData);
    };

}


