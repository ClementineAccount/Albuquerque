#pragma once
#include <Albuquerque/Application.hpp>
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

#include <optional>

#include "CarGame/SceneLoader.h"
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

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
    struct Sphere
    {
        glm::vec3 center;
        float radius;
    };

    static bool sphereCollisionCheck(Sphere const& lhs, Sphere const& rhs)
    {
        glm::vec3 temp = lhs.center - rhs.center;
        return (glm::dot(temp, temp)) < ((lhs.radius + rhs.radius) * (lhs.radius + rhs.radius));
    }

    static void SyncSphere(Sphere& sphere, glm::vec3 pos)
    {
        sphere.center = pos;
    }

    struct AABB 
    {
      glm::vec3 halfExtents{1.0f, 1.0f, 1.0f};
      glm::vec3 center{0.0f, 0.0f, 0.0f};
    };

    static bool AABBCollisionCheck(AABB const& lhs, AABB const& rhs) 
    {
      if (abs(lhs.center.x - rhs.center.x) >
          (lhs.halfExtents.x + rhs.halfExtents.y))
        return false;

      if (abs(lhs.center.y - rhs.center.y) >
          (lhs.halfExtents.y + rhs.halfExtents.y))
        return false;

      if (abs(lhs.center.z - rhs.center.z) >
          (lhs.halfExtents.z + rhs.halfExtents.z))
        return false;
    }

    //To Do: Write unit tests for the collision detection

    static void SyncAABB(Collision::AABB& aabb, glm::vec3 pos)
    {
        aabb.center = pos;
    }
}



class CarApplication final : public Application
{
public:
    static std::string LoadFile(std::string_view path);
    ~CarApplication();

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene() override;
    void RenderUI(double dt) override;
    void Update(double dt) override;
private:

    void LoadBuffers();

//To Do: This stuff should probably pass in the line vertex buffer it wants to subData()

//Adds the line to the specified buffer that is then draw I need to find a better name for this tbh
 void AddCollisionDrawLine(glm::vec3 ptA, glm::vec3 ptB, glm::vec3 color);
 void DrawLineAABB(Collision::AABB const& aabb, glm::vec3 boxColor);
 void DrawLineSphere(Collision::Sphere const& sphere, glm::vec3 sphereColor);

 //Can call this to reset the collision count in order to call 'DrawLine' every frame without creating new buffers
 void ClearLines();



private:
    
    std::optional<Fwog::GraphicsPipeline> pipeline_lines;
    std::optional<Fwog::GraphicsPipeline> pipeline_textured;
    std::optional<Fwog::GraphicsPipeline> pipeline_flat;

    static constexpr float axisScale = 1000.0f;
    static constexpr float PI = 3.1415926f;

    //Camera Stuff
    struct GlobalUniforms
    {
        glm::mat4 viewProj;
    };
    std::optional<Fwog::TypedBuffer<GlobalUniforms>> globalUniformsBuffer;

    static constexpr uint32_t num_points_world_axis = 6;

    static constexpr glm::vec3 worldOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    static constexpr glm::vec3 worldRight = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldForward = glm::vec3(0.0f, 0.0f, 1.0f);
    static constexpr float nearPlane = 0.01f;
    static constexpr float farPlane = 1000.0f;

    //World Axis stuff

    // Follows the same colors as https://docs.unity3d.com/ScriptReference/Transform.html
    static constexpr glm::vec3 worldUpColor = glm::vec3(0.0f, 1.0f, 0.0f);
    static constexpr glm::vec3 worldRightcolor = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 worldForwardColor = glm::vec3(0.0f, 0.0f, 1.0f);
    std::optional<Fwog::Buffer> vertex_buffer_pos_line;
    std::optional<Fwog::Buffer> vertex_buffer_color_line;

    static constexpr glm::vec3 skyColorDefault{ 0.1f, 0.3f, 0.5f };
    glm::vec3 skyColor{ skyColorDefault };


    //Objects in the world

    struct ObjectUniforms
    {
        glm::mat4 model;
        glm::vec4 color;
    };


    //Ground Plane Stuff
    //Could these live in the same data?
    static constexpr glm::vec3 planeScale = glm::vec3(1000.0f, 1.0f, 1000.0f);
    std::optional<Fwog::Buffer> vertex_buffer_plane;
    std::optional<Fwog::Buffer> index_buffer_plane;
    std::optional<Fwog::Texture> groundAlbedo;
    std::optional<Fwog::Buffer> objectBufferPlane;

    //Car Stuff
    float car_speed_scale{ 40.0f };
    float car_speed_scale_reverse{ 10.0f };

    // car's rotation when turning relative to the z-axis forward 
    float car_angle_turning_degrees{ 80.0f };
    float car_angle_degrees{ 0.0f };

    static constexpr glm::vec4 carColor{ 0.0f, 0.8f, 0.0f, 1.0f };
    static constexpr glm::vec4 wheelColor{ 0.5f, 0.5f, 0.5f, 1.0f };

    //not used yet
    //glm::vec3 wheelForward{ worldForward };

    glm::vec3 carForward{ worldForward };
    glm::vec3 carPos{ 0.0f, 0.0f, 0.0f };
    glm::vec3 carScale{1.0f, 1.0f, 1.0f};

    //Multiply with the carScale
    glm::vec3 carCollisionScale{1.5f, 1.2f, 1.5f};
    Collision::AABB car_box_collider;

    float car_sphere_radius = 1.0f;
    Collision::Sphere car_sphere_collider;

    static constexpr glm::vec3 cameraOffset = glm::vec3(0.0f, 10.0f, -12.0f);
    static constexpr glm::vec3 cameraOffsetTarget = glm::vec3(0.0f, 10.0f, 0.0f);
    static constexpr float soloud_volume{0.1f};


    //For loading the car from gltf file. Car and wheels as separate models (gotta implement some kind of skinned hirerarchy stuff otherwise)
    Utility::Scene scene_car;
    Utility::Scene scene_wheels;

    std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferCar;
    std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferWheels;

    SoLoud::Soloud soloud; 
    SoLoud::Wav sample;


    //Collision related stuff. Need to refactor
   
    //Collision Drawing
    static constexpr uint32_t max_num_collision_points = 65536;
    uint32_t curr_num_collision_points = 0;

    std::optional<Fwog::Buffer> vertex_buffer_collision_lines;
    std::optional<Fwog::Buffer> vertex_buffer_collision_colors;

};