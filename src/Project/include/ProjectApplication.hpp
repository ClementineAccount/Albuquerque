#pragma once
#include <Project.Library/Application.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
#include <iostream>


#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>



#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <optional>

#include "SceneLoader.h"


#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

// Jolt includes

#include <Jolt/Jolt.h>

//(JPH_ASSERT needs this)
using JPH::uint;
using JPH::AssertLastParam;

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <Jolt/Core/IssueReporting.h>

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

namespace MyCollision
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



// JPH activation listener (https://github.com/jrouwe/JoltPhysics/blob/master/HelloWorld/HelloWorld.cpp)
class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
	virtual void OnBodyActivated(const JPH::BodyID &inBodyID, uint64_t inBodyUserData) override;

	virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID, uint64_t inBodyUserData) override;
};




namespace PhysicsLayers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};  // namespace PhysicsLayers

class MyObjectLayerPairFilter : public JPH::ObjectLayerPairFilter {
 public:
  virtual bool ShouldCollide(JPH::ObjectLayer inObject1,
                             JPH::ObjectLayer inObject2) const override {
    switch (inObject1) {
      case PhysicsLayers::NON_MOVING:
        return inObject2 ==
               PhysicsLayers::MOVING;  // Non moving only collides with moving
      case PhysicsLayers::MOVING:
        return true;  // Moving collides with everything
      default:
        JPH_ASSERT(false);
        return false;
    }
  }
};

namespace BroadPhaseLayers {

static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr JPH::uint NUM_LAYERS(2);

};  // namespace BroadPhaseLayers

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class MyBPlayerInterface final : public JPH::BroadPhaseLayerInterface {
 public:
  MyBPlayerInterface() {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[PhysicsLayers::NON_MOVING] =
        BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[PhysicsLayers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual JPH::uint GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(
      JPH::ObjectLayer inLayer) const override {
    JPH_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *GetBroadPhaseLayerName(
      JPH::BroadPhaseLayer inLayer) const override {
    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
        return "NON_MOVING";
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
        return "MOVING";
      default:
        JPH_ASSERT(false);
        return "INVALID";
    }
  }
#endif  // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

 private:
  JPH::BroadPhaseLayer mObjectToBroadPhase[PhysicsLayers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class MyObjectVsBroadPhaseLayerFilter
    : public JPH::ObjectVsBroadPhaseLayerFilter {
 public:
  virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
                             JPH::BroadPhaseLayer inLayer2) const override {
    switch (inLayer1) {
      case PhysicsLayers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;
      case PhysicsLayers::MOVING:
        return true;
      default:
        JPH_ASSERT(false);
        return false;
    }
  }
};

// An example contact listener
class MyContactListener : public JPH::ContactListener {
 public:
  virtual JPH::ValidateResult OnContactValidate(
      const JPH::Body &inBody1, const JPH::Body &inBody2,
      JPH::RVec3Arg inBaseOffset,
      const JPH::CollideShapeResult &inCollisionResult) override {
    std::cout << "Contact validate callback" << std::endl;

    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  virtual void OnContactAdded(const JPH::Body &inBody1,
                              const JPH::Body &inBody2,
                              const JPH::ContactManifold &inManifold,
                              JPH::ContactSettings &ioSettings) override {
    std::cout << "A contact was added" << std::endl;
    std::cout << "inBody1.GetID().GetIndex(): " << inBody1.GetID().GetIndex() << std::endl;
  }

  virtual void OnContactPersisted(const JPH::Body &inBody1,
                                  const JPH::Body &inBody2,
                                  const JPH::ContactManifold &inManifold,
                                  JPH::ContactSettings &ioSettings) override {
    std::cout << "A contact was persisted" << std::endl;
  }

  virtual void OnContactRemoved(
      const JPH::SubShapeIDPair &inSubShapePair) override {
    std::cout << "A contact was removed" << std::endl;
  }


};


class ProjectApplication final : public Application
{
public:
    static std::string LoadFile(std::string_view path);
    ~ProjectApplication();

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene() override;
    void RenderUI(double dt) override;
    void Update(double dt) override;



private:

    // Going to try and NIH my Physics and Collision Detection instead. Its just
    // not as fun using a library makes me feel like I might as well use Godot
    void LoadJPH();
    void UpdateJPH();
    void UnloadJPH();

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

    static constexpr glm::vec3 cameraOffset = glm::vec3(0.0f, 10.0f, -12.0f);
    static constexpr glm::vec3 cameraOffsetTarget = glm::vec3(0.0f, 10.0f, 0.0f);


    static constexpr float soloud_volume{1.0f};

    //For loading the car from gltf file. Car and wheels as separate models (gotta implement some kind of skinned hirerarchy stuff otherwise)
    Utility::Scene scene_car;
    Utility::Scene scene_wheels;

    std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferCar;
    std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferWheels;

    // Declare some variables
    SoLoud::Soloud soloud; // Engine core
    SoLoud::Wav sample;    // One sample

    std::unique_ptr<JPH::PhysicsSystem> jph_physics_system;

    	// Create mapping table from object layer to broadphase layer
    // Note: As this is an interface, PhysicsSystem will take a reference to
    // this so this instance needs to stay alive!
    MyBPlayerInterface broad_phase_layer_interface;
	MyObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
	MyObjectLayerPairFilter object_vs_object_layer_filter;

    MyBodyActivationListener body_activation_listener;
    MyContactListener contact_listener;

    //Non owning pointer to body_interface inside physics system
    JPH::BodyInterface* body_interface;

    //Some placeholder physics bodies 

    //Non-owning (I think I have to refactor this to be some kind of shared_ptr?)
    JPH::Body *car_box_body;
    JPH::Body *test_box_body;
    JPH::Body *floor_body;

    std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;    
    std::unique_ptr<JPH::JobSystemThreadPool> job_system;


    static constexpr float kPhysicsDeltaTime = 1.0f / 60.0f;
};