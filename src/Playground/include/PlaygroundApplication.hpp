#pragma once

#include <Albuquerque/Application.hpp>
#include <Albuquerque/Camera.hpp>
#include <Albuquerque/DrawObject.hpp>


#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
#include <array>
#include <optional>


//Fwog Stuff
#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>


struct Skybox
{
    Skybox();

    std::optional<Fwog::Buffer> vertexBuffer;
    std::optional<Fwog::Texture> texture;
    std::optional<Fwog::GraphicsPipeline> pipeline;

    static Fwog::GraphicsPipeline MakePipleine(std::string_view, std::string_view);
    static Fwog::Texture MakeTexture();
};


//Could potentially rename it
struct GameObject
{
    void UpdateDraw();

    //Could refactor this data into a Transform struct
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    //Transformation only expected along one axis so for example purpose this is ok for now.
    glm::vec3 eulerAngleDegrees = glm::vec3(0.0f, 0.0f, 0.0f);

    Albuquerque::FwogHelpers::DrawObject drawData;
};

struct ViewData
{
    ViewData();

    struct ViewUniform {
        glm::mat4 viewProj;
        glm::vec3 eyePos;
    };

    std::optional<Fwog::TypedBuffer<ViewUniform>> viewBuffer;

    //Skybox doesn't have translation
    std::optional<Fwog::TypedBuffer<ViewUniform>> skyboxBuffer;

    void Update(Albuquerque::Camera const& camera);
};


struct Transform
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};

namespace VoxelStuff
{
    //So this is like one voxel object. Right now it naively uses one draw call per voxel
    //which I predict will have tons of performance issues very quickly, but we gonna avoid premature optimization
    struct Voxel
    {
        Voxel(Transform transform = Transform()); //Imagine making a default constructor (cant be me)
        GameObject gameObject;
        
        //Transform transform;
        
        //This is here because each voxel has its own model matrix (need to change that later)
        //Albuquerque::FwogHelpers::DrawObject drawData;

        void Draw() const;
    };

    struct Grid
    {
        Grid();

        // Lets try just an array of Voxels first
        // except I am going to make it a vector because of really good reasons:
        // (1): the big lazy

        static size_t constexpr numVoxelMax = 10;
        std::vector<Voxel> voxelGrid;

        //Voxel draw data called inside Draw (Not anymore)
        //Albuquerque::FwogHelpers::DrawObject drawDataVoxel;
        void Draw();
    };

}




class PlaygroundApplication final : public Albuquerque::Application
{
public:

    //Can probably move these both to a different class
    static Fwog::GraphicsPipeline MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
    static Fwog::Texture MakeTexture(std::string_view texturePath, int32_t expectedChannels = 4);

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene(double dt) override;
    void RenderUI(double dt) override;
    void Update(double dt) override;

    //void UpdateViewBuffers(Albuquerque::Camera const& camera);


protected:

    //Fwog related stuff (need to refactor further into their own classes in the future)

    bool LoadFwog();
    void RenderFwog(double dt);
    void UpdateFwog(double dt);


private:
   

    std::optional<Fwog::GraphicsPipeline> pipelineTextured_;
    std::optional<Fwog::Texture> cubeTexture_;
    Albuquerque::Camera sceneCamera_;

    std::optional<ViewData> viewData_;

    static constexpr size_t numCubes_ = 5;
    GameObject exampleCubes_[numCubes_];

    std::optional<Skybox> skybox_;

    bool skyboxVisible_ = false;

    bool fwogScene_ = true;
};