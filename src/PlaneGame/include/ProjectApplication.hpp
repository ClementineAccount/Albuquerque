#pragma once
#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

#include <Albuquerque/Application.hpp>
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "SceneLoader.h"
#include "ConfigReader.h"

namespace PlaneGame {

// To Do: Refactor this
namespace Primitives {

//This is the exact same as the Utility one and should be deprecated
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

static constexpr std::array<Vertex, 4> plane_vertices{
    // top (+y) (took from the cube)
    Vertex{{-0.5, 0.0, 0.5}, {0, 1, 0}, {0, 0}},
    {{0.5, 0.0, 0.5}, {0, 1, 0}, {1, 0}},
    {{0.5, 0.0, -0.5}, {0, 1, 0}, {1, 1}},
    {{-0.5, 0.0, -0.5}, {0, 1, 0}, {0, 1}}};

static constexpr std::array<uint32_t, 6> plane_indices{0, 1, 2, 2, 3, 0};

// Took it from 02_deferred.cpp lol
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

// Lazy so I stole it from the cursed LOGL
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
static constexpr std::array<float, 3 * 6 * 6> skybox_vertices = {

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
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

static constexpr std::array<uint32_t, 36> cube_indices{
    0,  1,  2,  2,  3,  0,

    4,  5,  6,  6,  7,  4,

    8,  9,  10, 10, 11, 8,

    12, 13, 14, 14, 15, 12,

    16, 17, 18, 18, 19, 16,

    20, 21, 22, 22, 23, 20,
};
}  // namespace Primitives

// To Do: Refactor this
namespace Collision {
// we are gonna go real mininium viable product first
struct Sphere {
  glm::vec3 center;
  float radius;
};

static bool sphereCollisionCheck(Sphere const& lhs, Sphere const& rhs) {
  glm::vec3 temp = lhs.center - rhs.center;
  return (glm::dot(temp, temp)) <
         ((lhs.radius + rhs.radius) * (lhs.radius + rhs.radius));
}

static void SyncSphere(Sphere& sphere, glm::vec3 pos) { sphere.center = pos; }

struct AABB {
  glm::vec3 halfExtents{1.0f, 1.0f, 1.0f};
  glm::vec3 center{0.0f, 0.0f, 0.0f};

  // because I don't need the half-points for Syncing, only for Sphere on AABB
  // checks
  inline glm::vec3 get_max_point() const { return center + halfExtents; };
  inline glm::vec3 get_min_point() const { return center - halfExtents; };
};

static bool AABBCollisionCheck(AABB const& lhs, AABB const& rhs) {
  if (abs(lhs.center.x - rhs.center.x) >
      (lhs.halfExtents.x + rhs.halfExtents.x))
    return false;

  if (abs(lhs.center.y - rhs.center.y) >
      (lhs.halfExtents.y + rhs.halfExtents.y))
    return false;

  if (abs(lhs.center.z - rhs.center.z) >
      (lhs.halfExtents.z + rhs.halfExtents.z))
    return false;
}

static bool CheckPointOnAABB(glm::vec3 const& point, AABB const& aabb) {
  glm::vec3 maxPoint = aabb.get_max_point();
  glm::vec3 minPoint = aabb.get_min_point();

  // Early rejection
  if (point.x > maxPoint.x || point.x < minPoint.x || point.y > maxPoint.y ||
      point.y < minPoint.y || point.z > maxPoint.z || point.z < minPoint.z) {
    return false;
  }

  return true;
}

static bool SphereAABBCollisionCheck(Sphere const& sphere, AABB const& aabb);

// To Do: Write unit tests for the collision detection

static void SyncAABB(Collision::AABB& aabb, glm::vec3 pos) {
  aabb.center = pos;
}

// static  bool RaycastCheck(glm::vec3 startPosition, glm::vec3 normalizedRay,
// AABB const& aabb, float stepDistance = 1.0f, size_t numSteps = 1000)
//{
//     //Early rejection test. (Disregard AABBs that are are not in the
//     direction of the ray) glm::vec3 dir = aabb.center - startPosition; if
//     (glm::dot(dir, normalizedRay) < 0.0f)
//     {
//         return false;
//     }

//    glm::vec3 currPosition = startPosition;
//    for (size_t i = 0; i < numSteps; ++i)
//    {
//        currPosition += (i * stepDistance) * normalizedRay;
//        if (CheckPointOnAABB(currPosition, aabb))
//            return true;
//    }

//    return false;
//}
}  // namespace Collision



class DrawCall
{
public:
    
    //DrawCall() = delete;
    //DrawCall(Fwog::Buffer auto const& vertex_buffer, Fwog::Buffer auto const& index_buffer);


    void SetBuffers(Fwog::Buffer const& vertex_buffer, Fwog::Buffer const& index_buffer);
    void SetModelTransformation(glm::mat4 const& model);
    void SetColor(glm::vec4 color);
    void Draw(uint64_t stride, uint32_t index_buffer_index = 0, uint32_t object_buffer_index = 1) const;

private:
    struct ObjectUniforms {
        glm::mat4 model{1.0f};
        glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};
    };

    ObjectUniforms uniform;
    Fwog::TypedBuffer<ObjectUniforms> object_buffer{Fwog::BufferStorageFlag::DYNAMIC_STORAGE};

    //To Do: Have this the keys to IDs of a buffer that are mapped by some resource manager
    Fwog::Buffer const* vertex_buffer = nullptr;
    Fwog::Buffer const* index_buffer = nullptr;
};

class Aircraft
{

};


class ProjectApplication final : public Albuquerque::Application {
 public:
  static std::string LoadFile(std::string_view path);
  ~ProjectApplication();

 protected:
  void AfterCreatedUiContext() override;
  void BeforeDestroyUiContext() override;
  bool Load() override;

  void ResetLevel();

  // For restarting and starting the game
  void StartLevel();

  void RenderScene() override;
  void RenderMousePick();

  void RenderUI(double dt) override;
  void Update(double dt) override;

  void UpdateEditorCamera(double dt);

  void MuteBackgroundMusicToggle(bool set_muted);
  void SetBackgroundMusic(SoLoud::Wav& bgm);

 private:
  void LoadBuffers();
  void LoadGroundPlane();

  void LoadCollectables();

  // Adds a collectable to the current scene
  void AddCollectable(glm::vec3 position,
                      glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f},
                      glm::vec3 color = glm::vec3{0.0f, 0.0f, 0.8f});

  // Adds the line to the specified buffer that is then draw I need to find a
  // better name for this tbh
  void AddDebugDrawLine(glm::vec3 ptA, glm::vec3 ptB, glm::vec3 color);
  void DrawLineAABB(Collision::AABB const& aabb, glm::vec3 boxColor);
  void DrawLineSphere(Collision::Sphere const& sphere, glm::vec3 sphereColor);

  // Can call this to reset the collision count in order to call 'DrawLine'
  // every frame without creating new buffers
  void ClearLines();

  void LoadBuildings();
  void AddBuilding(glm::vec3 position,
                   glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f},
                   glm::vec3 color = glm::vec3{1.0f, 0.0f, 0.0f});

  void CreateGroundChunks();

  Fwog::GraphicsPipeline CreatePipelineSkybox();
  void CreateSkybox();

  void AddCheckpoint(glm::vec3 position, glm::mat4 transform = glm::mat4{1.0f},
                     glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f},
                     float pitch_degrees = 0.0f, float yaw_degrees = 0.0f);
  void LoadCheckpoints();
  // void ClearCheckpoints();

  static float lerp(float start, float end, float t);

 private:

  enum class game_states : int32_t { playing, game_over, level_editor };

  game_states curr_game_state = game_states::playing;
  game_states prev_game_state = game_states::playing;

  std::optional<Fwog::GraphicsPipeline> pipeline_lines;
  std::optional<Fwog::GraphicsPipeline> pipeline_textured;
  std::optional<Fwog::GraphicsPipeline> pipeline_flat;

  std::optional<Fwog::GraphicsPipeline> pipeline_colored_indexed;

  std::optional<Fwog::GraphicsPipeline> pipeline_skybox;

  // Draw with arrays and not indexed so we don't need indices
  std::optional<Fwog::Buffer> vertex_buffer_skybox;

  static constexpr float axisScale = 1000.0f;
  static constexpr float PI = 3.1415926f;


  ConfigReader configInstance;

  // Some ideas of a score system

  float current_player_level_time = 0.0f;
  float personal_best = 100.0f;  // some absurdly high number
  static constexpr float bronze_level_time = 20.0f;
  static constexpr float silver_level_time = 15.0f;
  static constexpr float gold_level_time = 13.0f;

  std::map<float, std::string> medal_times;

  std::string player_medal_name = "None";
  float player_medal_timing = 100.0f;

  // struct medal_timing_pair
  //{
  //     const char* medal_name;
  //     float medal_timing = 0.0f;
  // };

  // Camera Stuff
  struct GlobalUniforms {
    glm::mat4 viewProj;
    glm::vec3 eyePos;
  };
  std::optional<Fwog::TypedBuffer<GlobalUniforms>> globalUniformsBuffer;
  std::optional<Fwog::TypedBuffer<GlobalUniforms>> globalUniformsBuffer_skybox;

  GlobalUniforms globalStruct;

  static constexpr uint32_t num_points_world_axis = 6;

  static constexpr glm::vec3 worldOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
  static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
  static constexpr glm::vec3 worldRight = glm::vec3(1.0f, 0.0f, 0.0f);
  static constexpr glm::vec3 worldForward = glm::vec3(0.0f, 0.0f, 1.0f);
  static constexpr float nearPlane = 0.01f;
  static constexpr float farPlane = 5000.0f;

  // World Axis stuff

  // Follows the same colors as
  // https://docs.unity3d.com/ScriptReference/Transform.html
  static constexpr glm::vec3 worldUpColor = glm::vec3(0.0f, 1.0f, 0.0f);
  static constexpr glm::vec3 worldRightcolor = glm::vec3(1.0f, 0.0f, 0.0f);
  static constexpr glm::vec3 worldForwardColor = glm::vec3(0.0f, 0.0f, 1.0f);
  std::optional<Fwog::Buffer> vertex_buffer_pos_line;
  std::optional<Fwog::Buffer> vertex_buffer_color_line;

  static constexpr glm::vec3 skyColorDefault{0.1f, 0.3f, 0.5f};
  static constexpr glm::vec3 skyColorFoggy{0.04519f, 0.05781f, 0.09084f};
  glm::vec3 skyColor{skyColorFoggy};

  // Objects in the world
  struct ObjectUniforms {
    // Currently I set the scale to 0.0f if I want to not render an object
    // uniform that is indexed but there has to be an alterative way
    glm::mat4 model;
    glm::vec4 color;
  };

  // Ground Plane Stuff
  // Could these live in the same data?
  static constexpr glm::vec3 planeScale = glm::vec3(4000.0f, 1.0f, 4000.0f);
  // static constexpr glm::vec3 planeScale = glm::vec3(100.0f, 1.0f, 100.0f);
  std::optional<Fwog::Buffer> vertex_buffer_plane;
  std::optional<Fwog::Buffer> index_buffer_plane;
  std::optional<Fwog::Texture> groundAlbedo;
  std::optional<Fwog::Buffer> objectBufferPlane;

  std::optional<Fwog::Texture> skybox_texture;

  // Want to test multiple ground chunks
  struct ground_chunk {
    std::optional<Fwog::Buffer> object_buffer;

    ObjectUniforms ground_uniform;
    glm::vec3 ground_center{0.0f, 0.0f, 0.0f};

    // To DO: albedo IDs?
  };

  std::vector<ground_chunk> grond_chunk_list;

  // aircraft stuff
  struct PhysicsBody {
    float current_speed = 0.0f;

    glm::vec3 forward_vector{0.0f, 0.0f, 1.0f};
    glm::vec3 up_vector{0.0f, 1.0f, 0.0f};
    glm::vec3 right_vector{1.0f, 0.0f, 0.0f};

    glm::mat4 rotMatrix{1.0f};

    float propeller_angle_degrees = 0.0f;
  };

  float aircraft_max_speed = 300.0f;
  float aircraft_min_speed = 40.0f;
  static constexpr float aircraft_speed_increase_per_second = 30.0f;
  static constexpr float aircraft_speed_decrease_per_second = -30.0f;

  static constexpr float max_zoom_level_scale = 1.3f;
  static constexpr float min_zoom_level_scale = 1.0f;

  static constexpr float base_fov_degrees = 90.0f;
  static constexpr float base_fov_radians = (base_fov_degrees * PI / 180.0f);

  constexpr static float aircraft_starting_speed{50.0f};
  constexpr static glm::vec3 aircraft_starting_direction_vector{0.0f, 0.0f,
                                                                1.0f};

  PhysicsBody aircraft_body{aircraft_starting_speed,
                            aircraft_starting_direction_vector};

  // convering RPM to degrees per second
  // (https://www.infoconverter.com/conversion/frequency/revolutions-per-minute-to-degrees-per-second)
  constexpr static float propeller_angle_turning_degrees = -1000.0f;

  float elasped_propeller_time = 0.0f;
  constexpr static float propeller_revolutions_per_second =
      720.0f / 60.0f;  // maximium value

  float aircraft_speed_scale{3.0f};
  float aircraft_speed_scale_reverse{10.0f};

  constexpr static float aircraft_speedup_scale{30.0f};
  float aircraft_current_speed_scale{1.0f};

  // aircraft's rotation when turning relative to the z-axis forward (per second
  // of course)
  float aircraft_angle_turning_degrees{60.0f};

  static constexpr glm::vec4 aircraftColor{0.0f, 0.8f, 0.0f, 1.0f};
  static constexpr glm::vec4 wheelColor{0.5f, 0.5f, 0.5f, 1.0f};

  static constexpr float aircraft_max_roll_degrees{60.0f};

  bool render_plane = true;

  static constexpr glm::vec3 aircarftStartPos{0.0f, 65.0f, 0.0f};

  glm::vec3 aircraftForward{worldForward};
  glm::vec3 aircraftPos{aircarftStartPos};
  glm::vec3 aircraftScale{1.0f, 1.0f, 1.0f};

  // Multiply with the aircraftScale
  glm::vec3 aircraftCollisionScale{1.5f, 1.2f, 1.5f};
  Collision::AABB aircraft_box_collider;

  float aircraft_sphere_radius = 6.0f;
  Collision::Sphere aircraft_sphere_collider;

  static constexpr glm::vec3 cameraOffset = glm::vec3(0.0f, 10.0f, 20.0f);

  static constexpr glm::vec3 cameraOffsetTarget = glm::vec3(0.0f, 10.0f, 0.0f);
  static constexpr float soloud_volume{1.0f};

  // For loading the aircraft from gltf file. aircraft and wheels as separate
  // models (gotta implement some kind of skinned hirerarchy stuff otherwise)
  Utility::Scene scene_aircraft;
  Utility::Scene scene_wheels;

  std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferaircraft;
  std::optional<Fwog::TypedBuffer<ObjectUniforms>> object_buffer_propeller;

  std::optional<Fwog::TypedBuffer<ObjectUniforms>> objectBufferWheels;

  SoLoud::Soloud soloud;
  SoLoud::Wav sample;
  int plane_flying_sfx_handle;
  SoLoud::Wav plane_flying_sfx;
  SoLoud::Wav plane_speedup_sfx;
  SoLoud::Wav background_music;
  SoLoud::Wav level_editor_music;
  SoLoud::Wav collectable_pickup_sfx;

  SoLoud::Wav* curr_backgrond_music = nullptr;

  bool is_background_music_muted = true;

  // Collision related stuff. Need to refactor

  // Collision Drawing
  static constexpr uint32_t max_num_draw_points = 65536;
  uint32_t curr_num_draw_points = 0;

  std::optional<Fwog::Buffer> vertex_buffer_draw_lines;
  std::optional<Fwog::Buffer> vertex_buffer_draw_colors;

  // Spheres
  struct collectable {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    bool isCollected = false;

    Collision::Sphere collider{position, scale.x};
  };

  std::vector<collectable> collectableList;

  // Drawing with instancing
  Utility::Scene scene_collectable;
  std::optional<Fwog::TypedBuffer<ObjectUniforms>> collectableObjectBuffers;

  // How many instances to draw
  // uint32_t num_active_collectables{0};
  static constexpr uint32_t max_num_collectables{4096};

  bool renderAxis = false;
  bool draw_collectable_colliders = false;
  bool draw_player_colliders = false;

  static constexpr glm::vec3 default_building_color{0.29614, 0.43966, 0.52712};

  struct buildingObject {
    glm::vec3 building_center{50.0f, 0.0f, 100.0f};
    glm::vec3 building_scale{10.0f, 100.0f, 10.0f};
    Collision::AABB building_collider{building_scale * 0.5f, building_center};

    DrawCall drawcall;

    static std::optional<Fwog::Texture> buildingAlbedo;
  };

  std::optional<Fwog::Buffer> building_vertex_buffer;
  std::optional<Fwog::Buffer> building_index_buffer;

  // buildingObject hello_building;
  std::vector<buildingObject> buildingObjectList;

  struct checkpointObject {
    static constexpr glm::vec3 activated_color_linear =
        glm::vec3(0.016f, 0.57758f, 0.00335f);
    static constexpr glm::vec3 non_activated_color_linear =
        glm::vec3(1.0f, 0.0f, 0.0f);

    // Based off the size of the actual model loaded in
    static constexpr float base_radius = 13.0f;

    glm::vec3 center;
    glm::vec3 scale;
    glm::vec3 color = non_activated_color_linear;

    glm::mat4 model;
    // glm::mat4 rotation_model_matrix{1.0f};

    Collision::Sphere collider;
    std::optional<Fwog::Buffer> object_buffer;

    // First on the list is activated followed by the next. The load order from
    // the level file is important
    bool activated = false;
  };

  // Think maybe putting it here is easier to organize? Its like using the
  // struct as a 'namespace'
  std::optional<Fwog::Buffer> checkpoint_vertex_buffer;
  std::optional<Fwog::Buffer> checkpoint_index_buffer;

  Utility::Scene scene_checkpoint_ring;

  size_t curr_active_checkpoint = 0;
  std::vector<checkpointObject> checkpointList;
  bool all_checkpoints_collected = false;

  struct camera {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    glm::vec3 forward;
    glm::vec3 right;
  };
  camera editorCamera;
  camera gameplayCamera;

  std::optional<Fwog::Texture> mousePick_Texture;
  void MouseRaycast(camera const& cam);

  // To Do: Rewrite this after fixing architecture to decouple collision,
  // rendering and entity data

  static buildingObject * RaycastCheck(
      glm::vec3 startPosition, glm::vec3 normalizedRay,
      std::vector<buildingObject> & aabb_list, float stepDistance = 1.0f,
      size_t numSteps = 1000) {
    // Reject all colliders that cannot possibly be hit
    std::vector<buildingObject*> non_rejected;
    for (auto& aabb : aabb_list) {
      glm::vec3 dir = aabb.building_collider.center - startPosition;
      if (glm::dot(dir, normalizedRay) > 0.0f) {
        non_rejected.push_back(&aabb);
      }
    }

    glm::vec3 currPosition = startPosition;
    for (size_t i = 0; i < numSteps; ++i) {
      currPosition += (i * stepDistance) * normalizedRay;

      // Check if there is a hit for every step. If there is simply return that
      // and negate the rest
      for (auto & aabb : non_rejected) {
        if (CheckPointOnAABB(currPosition, aabb->building_collider))
          return aabb;
      }
    }

    return nullptr;
  }
};

}  // namespace PlaneGame
