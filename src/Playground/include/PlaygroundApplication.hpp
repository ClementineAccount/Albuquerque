#pragma once

#include <Albuquerque/Application.hpp>
#include <Albuquerque/Camera.hpp>
#include <Albuquerque/DrawObject.hpp>
#include <Voxel.hpp>

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





//More for debug lines and stuff I guess? Can have multiple instances. I'll make this a class later
class LineRendererFwog
{
public:
    LineRendererFwog();

    void AddPoint(glm::vec3 point_position, glm::vec3 point_color = default_line_color);
    void Clear();

    //To Do: Decouple this from viewData
    void Draw(ViewData const& viewData);

private:

    static constexpr size_t maxPoints = 1024;
    static constexpr glm::vec3 default_line_color = glm::vec3(1.0f, 0.0f, 0.0f);

    std::optional<Fwog::GraphicsPipeline> pipeline;

    //These points are passed in in worldspace coordinates
    std::optional<Fwog::TypedBuffer<glm::vec3>> vertex_buffer;
    std::optional<Fwog::TypedBuffer<glm::vec3>> color_buffer;

    size_t point_count = 0;


    struct Tests
    {
        //Must be called after Fwog is initalized
        //static void TestAddRemove();
    };

};




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

    //TODO: Move these to their own helper functions
    
protected:
    //Testing out the concept of right click mouse movement
    void UpdateMouseOffset(double dt, double& xOffset, double& yOffset);

    //For mouse picking
    glm::vec3 RaycastScreenToWorld(Albuquerque::Camera const& cam);


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

    static constexpr size_t numCubes_ = 10;
    GameObject exampleCubes_[numCubes_];

    std::optional<Skybox> skybox_;

    bool skyboxVisible_ = false;

    bool fwogScene_ = true;
    std::optional<VoxelStuff::Grid> voxelGrid_;


    std::optional<LineRendererFwog> line_renderer;
};