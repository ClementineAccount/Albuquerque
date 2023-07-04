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


namespace Primitives
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    using indexType =  uint32_t;

    // Took it from fwog's examples 02_deferred.cpp
    static constexpr std::array<Vertex, 24> cubeVertices{
        // front (+z)
        Vertex{{-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}},
        {{0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0}},
        {{0.5, 0.5, 0.5}, {0, 0, 1}, {1, 1}},
        {{-0.5, 0.5, 0.5}, {0, 0, 1}, {0, 1}},

        // back (-z)
        {{-0.5, 0.5, -0.5}, {0, 0, -1}, {1, 1}},
        {{0.5, 0.5, -0.5}, {0, 0, -1}, {0, 1}},
        {{0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}},
        {{-0.5, -0.5, -0.5}, {0, 0, -1}, {1, 0}},

        // left (-x)
        {{-0.5, -0.5, -0.5}, {-1, 0, 0}, {0, 0}},
        {{-0.5, -0.5, 0.5}, {-1, 0, 0}, {1, 0}},
        {{-0.5, 0.5, 0.5}, {-1, 0, 0}, {1, 1}},
        {{-0.5, 0.5, -0.5}, {-1, 0, 0}, {0, 1}},

        // right (+x)
        {{0.5, 0.5, -0.5}, {1, 0, 0}, {1, 1}},
        {{0.5, 0.5, 0.5}, {1, 0, 0}, {0, 1}},
        {{0.5, -0.5, 0.5}, {1, 0, 0}, {0, 0}},
        {{0.5, -0.5, -0.5}, {1, 0, 0}, {1, 0}},

        // top (+y)
        {{-0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}},
        {{0.5, 0.5, 0.5}, {0, 1, 0}, {1, 0}},
        {{0.5, 0.5, -0.5}, {0, 1, 0}, {1, 1}},
        {{-0.5, 0.5, -0.5}, {0, 1, 0}, {0, 1}},

        // bottom (-y)
        {{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}},
        {{0.5, -0.5, -0.5}, {0, -1, 0}, {1, 0}},
        {{0.5, -0.5, 0.5}, {0, -1, 0}, {1, 1}},
        {{-0.5, -0.5, 0.5}, {0, -1, 0}, {0, 1}},
    };

    static constexpr std::array<indexType, 36> cubeIndices{
        0,  1,  2,  2,  3,  0,

        4,  5,  6,  6,  7,  4,

        8,  9,  10, 10, 11, 8,

        12, 13, 14, 14, 15, 12,

        16, 17, 18, 18, 19, 16,

        20, 21, 22, 22, 23, 20,
    };

    // https://learnopengl.com/Advanced-OpenGL/Cubemaps
    static constexpr std::array<float, 3 * 6 * 6> skyboxVertices = {

        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f
    };
}



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

private:

    std::optional<Fwog::GraphicsPipeline> pipelineTextured_;
    std::optional<Fwog::Texture> cubeTexture_;
    Albuquerque::Camera sceneCamera_;

    std::optional<ViewData> viewData_;

    static constexpr size_t numCubes_ = 5;
    GameObject exampleCubes_[numCubes_];

    std::optional<Skybox> skybox_;

    bool skyboxVisible_ = false;
};