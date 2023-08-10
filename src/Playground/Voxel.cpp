
#include <Voxel.hpp>

#include <Albuquerque/Primitives.hpp>
#include <Albuquerque/FwogHelpers.hpp>
#include <Albuquerque/DrawObject.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>

VoxelStuff::Voxel::Voxel(Transform transform)
{
    //Create the drawData based off a cube (..for now...)
    //using namespace Albuquerque;
    //gameObject.drawData = Albuquerque::FwogHelpers::DrawObject::Init("cube");

    transform.position = transform.position;
    transform.scale = transform.scale;

    //its like midnight and i dont remember if its row-order or column-order in glm so imma just do it the glm way
    //To Do: Just set the values in the matrix directly instead of calling the function
    glm::mat4 model(1.0f);

    //I keep forgetting that glm's TSR is reversed. So it really is T -> R - > S
    model = glm::translate(model, transform.position);
    model = glm::scale(model, transform.scale);

    objectUniform.modelTransform = model;

    //gameObject.UpdateDraw();
}

VoxelStuff::Grid::Grid()
{
    //Make the pipeline
    constexpr char vertexShaderPath[] = "";
    constexpr char fragmentShaderPath[] = "";

    //To Do: Move the PlaygroundApplication thing to its own file
    pipeline = Albuquerque::FwogHelpers::MakePipeline("./data/shaders/voxel.vs.glsl", "./data/shaders/voxel.fs.glsl");


    //Get the voxel mesh and bind it
    constexpr char meshName[] = "cube";
    voxelMeshBufferRef = &Albuquerque::FwogHelpers::MeshBuffer::GetMap()[meshName];

    //Create a row of voxels with an offset
    float distanceOffset = 1.05f;

    glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 currPos = startPosition;

    //for (size_t i = 0; i < numVoxelMax; ++i)
    //{
    //    voxelGrid.emplace_back(VoxelStuff::Voxel(Transform(currPos)));
    //    currPos.x += distanceOffset;
    //}

    //auto createGrid2D = [&](size_t numCol, size_t numRows)
    //{
    //    for (size_t r = 0; r < numRows; ++r)
    //    {
    //        for (size_t c = 0; c < numCol; ++c)
    //        {
    //            voxelGrid.emplace_back(VoxelStuff::Voxel(Transform(currPos)));
    //            objectUniforms.emplace_back(voxelGrid.back().objectUniform);
    //            currPos.x += distanceOffset;
    //        }
    //        currPos.x = 0;
    //        currPos.y -= distanceOffset;
    //    }
    //};


    auto createGrid3D = [&](size_t numCol, size_t numRows, size_t numStacks)
    {
        for (size_t s = 0; s < numStacks; ++s)
        {
            for (size_t r = 0; r < numRows; ++r)
            {
                for (size_t c = 0; c < numCol; ++c)
                {
                    voxelGrid.emplace_back(VoxelStuff::Voxel(Transform(currPos)));
                    //Consideration: If we do removal whether we could remove in place then push back the coordinate
                    objectUniforms.emplace_back(voxelGrid.back().objectUniform);
                    currPos.x += distanceOffset;
                }
                currPos.x = 0;
                currPos.z -= distanceOffset;
            }
            currPos.x = 0;
            currPos.z = 0;
            currPos.y -= distanceOffset;
        }
    };

    //createGrid2D(10, 2);
    createGrid3D(300, 300, 5);

    objectBuffer.emplace(std::span(objectUniforms), Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

}

void VoxelStuff::Grid::Draw(Fwog::Texture const& textureAlbedo, Fwog::Sampler const& sampler, ViewData const& viewData)
{
    //for (auto const& voxel : voxelGrid)
    //{
    //    voxel.Draw();
    //}

    //To Do: Bind pipeline here

    Fwog::Cmd::BindGraphicsPipeline(pipeline.value());
    Fwog::Cmd::BindUniformBuffer(0, viewData.viewBuffer.value());
    Fwog::Cmd::BindStorageBuffer(1, *objectBuffer);

    Fwog::Cmd::BindSampledImage(0, textureAlbedo, sampler);


    Fwog::Cmd::BindVertexBuffer(0, *voxelMeshBufferRef->vertexBuffer, 0, sizeof(Albuquerque::Primitives::Vertex));
    Fwog::Cmd::BindIndexBuffer(*voxelMeshBufferRef->indexBuffer, Fwog::IndexType::UNSIGNED_INT);
    Fwog::Cmd::DrawIndexed(voxelMeshBufferRef->indexCount, voxelGrid.size(), 0, 0, 0);
}

