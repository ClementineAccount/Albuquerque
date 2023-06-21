#pragma once

#include <Albuquerque/Application.hpp>

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

struct DrawObject
{
    //T1 and T2 can be different container types. std::array or std::vector.
    //Didn't want this to be a constructor because the actual DrawObject struct does not need to be templated.
    template <typename T1, typename T2>
    static DrawObject Init(T1 const& vertexList, T2 const& indexList, size_t indexCount);

    std::optional<Fwog::Buffer> vertexBuffer;
    std::optional<Fwog::Buffer> indexBuffer;

    uint32_t indexCount;

    struct ObjectUniform
    {
        glm::mat4 modelTransform = glm::mat4(1.0f);
    };
    ObjectUniform objectStruct;

    std::optional<Fwog::TypedBuffer<ObjectUniform>> modelUniformBuffer;
};

struct Skybox
{
    Skybox();

    std::optional<Fwog::Buffer> vertexBuffer;
    std::optional<Fwog::Texture> texture;
    std::optional<Fwog::GraphicsPipeline> pipeline;

    static Fwog::GraphicsPipeline MakePipleine(std::string_view, std::string_view);
    static Fwog::Texture MakeTexture();
};

struct Camera
{
    Camera();
    void Update();

    glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float nearPlane = 0.01f;
    float farPlane = 5000.0f;
    
    struct CameraUniforms {
        glm::mat4 viewProj;
        glm::vec3 eyePos;
    };

    CameraUniforms cameraStruct;

    std::optional<Fwog::TypedBuffer<CameraUniforms>> cameraUniformsBuffer;

    //Skybox doesn't pass in the translation
    std::optional<Fwog::TypedBuffer<CameraUniforms>> cameraUniformsSkyboxBuffer;
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

    DrawObject drawData;
};

class ProjectApplication final : public Albuquerque::Application
{
public:

    //Can probably move these both to a different class
    static Fwog::GraphicsPipeline MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
    static Fwog::Texture MakeTexture(std::string_view texturePath, int32_t expectedChannels = 4);

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene() override;
    void RenderUI(double dt) override;
    void Update(double dt) override;

private:
    //uint32_t shaderProgram;
    //bool MakeShader(std::string_view vertexShaderFilePath, std::string_view fragmentShaderFilePath);

    std::optional<Fwog::GraphicsPipeline> pipelineTextured;
    std::optional<Fwog::Texture> cubeTexture;
    std::optional<Camera> sceneCamera;

    static constexpr size_t numCubes = 5;
    GameObject exampleCubes[numCubes];

    std::optional<Skybox> skybox;

    bool _skyboxVisible = false;
};