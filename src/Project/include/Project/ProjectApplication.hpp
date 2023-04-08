#pragma once

#include <Project.Library/Application.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>

#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>


#include "common/SceneLoader.h"

#include <optional>



namespace Primitives
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };


    static constexpr std::array<Vertex, 4> plane_vertices{
        // top (+y) (took from the cube)
       Vertex{{-0.5, 0.0, 0.5}, {0, 1, 0}, {0, 0}},
       {{0.5, 0.0, 0.5}, {0, 1, 0}, {1, 0}},
       {{0.5, 0.0, -0.5}, {0, 1, 0}, {1, 1}},
       {{-0.5, 0.0, -0.5}, {0, 1, 0}, {0, 1}} };

    static constexpr std::array<uint16_t, 6> plane_indices{ 0, 1, 2, 2, 3, 0 };


    //Took it from 02_deferred.cpp lol
    static constexpr std::array<Vertex, 24> cube_vertices{
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

    static constexpr std::array<uint16_t, 36> cube_indices{
      0,  1,  2,  2,  3,  0,

      4,  5,  6,  6,  7,  4,

      8,  9,  10, 10, 11, 8,

      12, 13, 14, 14, 15, 12,

      16, 17, 18, 18, 19, 16,

      20, 21, 22, 22, 23, 20,
    };
}

namespace Collision
{
    //we are gonna go real mininium viable product first
    struct sphereCollider
    {
        glm::vec3 pos;
        float radius;
    };

    static bool sphereCollisionCheck(sphereCollider const& lhs, sphereCollider const& rhs)
    {
        glm::vec3 temp = lhs.pos - rhs.pos;
        return (glm::dot(temp, temp)) < ((lhs.radius + rhs.radius) * (lhs.radius + rhs.radius));
    }
}


class ProjectApplication final : public Application
{
public:
    static std::string LoadFile(std::string_view path);

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene() override;
    void RenderUI(double dt) override;
    void Update(double dt) override;

private:
    uint32_t _shaderProgram;
    
    std::optional<Fwog::GraphicsPipeline> pipeline_lines;
    std::optional<Fwog::GraphicsPipeline> pipeline;

    static constexpr float axisScale = 1000.0f;
    static constexpr float PI = 3.1415926f;

    static constexpr uint32_t num_points_world_axis = 6;

    static constexpr glm::vec3 worldOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    static constexpr glm::vec3 worldRight = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldForward = glm::vec3(0.0f, 0.0f, 1.0f);

    // Follows the same colors as https://docs.unity3d.com/ScriptReference/Transform.html
    static constexpr glm::vec3 worldUpColor = glm::vec3(0.0f, 1.0f, 0.0f);
    static constexpr glm::vec3 worldRightcolor = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldForwardColor = glm::vec3(0.0f, 0.0f, 1.0f);

    std::optional<Fwog::Buffer> vertex_buffer_pos_line;
    std::optional<Fwog::Buffer> vertex_buffer_color_line;

    static constexpr float nearPlane = 0.01f;
    static constexpr float farPlane = 1000.0f;

    static constexpr glm::vec3 skyColorDefault{ 0.1f, 0.3f, 0.5f };
    glm::vec3 skyColor{ 1.0f, 0.3f, 0.5f };

    struct GlobalUniforms
    {
        glm::mat4 viewProj;
    };


    std::optional<Fwog::TypedBuffer<GlobalUniforms>> globalUniformsBuffer;
};