#include "ProjectApplication.hpp"

#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>

//This must be included before GLFW or else you'd get https://stackoverflow.com/questions/3927810/how-to-prevent-macro-redefinition
//https://web.archive.org/web/20230630052623/https://stackoverflow.com/questions/3927810/how-to-prevent-macro-redefinition

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

//https://stackoverflow.com/questions/44345811/glad-h-giving-error-that-opengl-header-is-included
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <iterator>
#include <queue>
#include <set>
#include <thread>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
#include <unordered_map>
#include <vector>

#include "SceneLoader.h"
#include "stb_image.h"

namespace PlaneGame {
    
    


static std::string Slurp(std::string_view path) {
  std::ifstream file(path.data(), std::ios::ate);
  std::string result(file.tellg(), '\0');
  file.seekg(0);
  file.read((char*)result.data(), result.size());
  return result;
}

namespace fs = std::filesystem;

static constexpr char vert_shader_path[] =
    "data/shaders/FwogRacing/hello_car.vert.glsl";
static constexpr char frag_shader_path[] =
    "data/shaders/FwogRacing/hello_car.frag.glsl";
static constexpr char frag_texture_shader_path[] =
    "data/shaders/FwogRacing/hello_car_textured.frag.glsl";

static constexpr char vert_line_shader_path[] =
    "data/shaders/FwogRacing/lines.vert.glsl";
static constexpr char frag_line_shader_path[] =
    "data/shaders/FwogRacing/lines.frag.glsl";

static constexpr char vert_indexed_shader_path[] =
    "data/shaders/draw_indexed.vert.glsl";
static constexpr char frag_color_shader_path[] = "data/shaders/color.frag.glsl";
static constexpr char frag_phong_shader_path[] =
    "data/shaders/phongFog.frag.glsl";

static constexpr char vert_skybox_shader_path[] =
    "data/shaders/skybox.vert.glsl";
static constexpr char frag_skybox_shader_path[] =
    "data/shaders/skybox.frag.glsl";

static bool Collision::SphereAABBCollisionCheck(Sphere const& sphere,
                                                AABB const& aabb) {
  using std::max;
  using std::min;

  glm::vec3 maxPoint = aabb.get_max_point();
  glm::vec3 minPoint = aabb.get_min_point();

  glm::vec3 nearestPointbox;
  nearestPointbox.x = max(minPoint.x, min(sphere.center.x, maxPoint.x));
  nearestPointbox.y = max(minPoint.y, min(sphere.center.y, maxPoint.y));
  nearestPointbox.z = max(minPoint.z, min(sphere.center.z, maxPoint.z));

  glm::vec3 center_to_point_box = nearestPointbox - sphere.center;
  return glm::dot(center_to_point_box, center_to_point_box) <
         (sphere.radius * sphere.radius);
}

std::string ProjectApplication::LoadFile(std::string_view path) {
  std::ifstream file{path.data()};
  return {std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>()};
}

static Fwog::GraphicsPipeline CreatePipeline() {
  // Specify our two vertex attributes: position and color.
  // Positions are 3x float, so we will use R32G32B32_FLOAT like we would in
  // Vulkan.
  static constexpr auto sceneInputBindingDescs = std::array{
      Fwog::VertexInputBindingDescription{
          // color
          .location = 0,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, position),
      },
      Fwog::VertexInputBindingDescription{
          // normal
          .location = 1,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, normal),
      },
      Fwog::VertexInputBindingDescription{
          // texcoord
          .location = 2,
          .binding = 0,
          .format = Fwog::Format::R32G32_FLOAT,
          .offset = offsetof(Primitives::Vertex, uv),
      },
  };

  auto inputDescs = sceneInputBindingDescs;
  auto primDescs =
      Fwog::InputAssemblyState{Fwog::PrimitiveTopology::TRIANGLE_LIST};

  auto vertexShader =
      Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER,
                   ProjectApplication::LoadFile(vert_shader_path));
  auto fragmentShader =
      Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER,
                   ProjectApplication::LoadFile(frag_phong_shader_path));

  return Fwog::GraphicsPipeline{{
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .depthState = {.depthTestEnable = true,
                     .depthWriteEnable = true,
                     .depthCompareOp = Fwog::CompareOp::LESS},
  }};
}

static Fwog::GraphicsPipeline CreatePipelineLines() {
  auto descPos = Fwog::VertexInputBindingDescription{
      .location = 0,
      .binding = 0,
      .format = Fwog::Format::R32G32B32_FLOAT,
      .offset = 0,
  };

  auto descColor = Fwog::VertexInputBindingDescription{
      .location = 1,
      .binding = 1,
      .format = Fwog::Format::R32G32B32_FLOAT,
      .offset = 0,
  };

  auto inputDescs = {descPos, descColor};

  auto primDescs = Fwog::InputAssemblyState{Fwog::PrimitiveTopology::LINE_LIST};
  auto depthDescs =
      Fwog::DepthState{.depthTestEnable = false, .depthWriteEnable = false};
  auto vertexShader =
      Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER,
                   ProjectApplication::LoadFile(vert_line_shader_path));
  auto fragmentShader =
      Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER,
                   ProjectApplication::LoadFile(frag_line_shader_path));

  return Fwog::GraphicsPipeline{{
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
      .depthState = {.depthTestEnable = false, .depthWriteEnable = false},
  }};
}

static Fwog::GraphicsPipeline CreatePipelineTextured() {
  // Specify our two vertex attributes: position and color.
  // Positions are 3x float, so we will use R32G32B32_FLOAT like we would in
  // Vulkan.
  static constexpr auto sceneInputBindingDescs = std::array{
      Fwog::VertexInputBindingDescription{
          // color
          .location = 0,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, position),
      },
      Fwog::VertexInputBindingDescription{
          // normal
          .location = 1,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, normal),
      },
      Fwog::VertexInputBindingDescription{
          // texcoord
          .location = 2,
          .binding = 0,
          .format = Fwog::Format::R32G32_FLOAT,
          .offset = offsetof(Primitives::Vertex, uv),
      },
  };

  auto inputDescs = sceneInputBindingDescs;
  auto primDescs =
      Fwog::InputAssemblyState{Fwog::PrimitiveTopology::TRIANGLE_LIST};

  auto vertexShader =
      Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER,
                   ProjectApplication::LoadFile(vert_shader_path));
  auto fragmentShader =
      Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER,
                   ProjectApplication::LoadFile(frag_texture_shader_path));

  return Fwog::GraphicsPipeline{{
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .depthState = {.depthTestEnable = true,
                     .depthWriteEnable = true,
                     .depthCompareOp = Fwog::CompareOp::LESS},
  }};
}

static Fwog::GraphicsPipeline CreatePipelineColoredIndex() {
  // To be honest since this is the same as the others I might as well just pass
  // in the shader paths instead

  static constexpr auto sceneInputBindingDescs = std::array{
      Fwog::VertexInputBindingDescription{
          // position
          .location = 0,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, position),
      },
      Fwog::VertexInputBindingDescription{
          // normal
          .location = 1,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = offsetof(Primitives::Vertex, normal),
      },
      Fwog::VertexInputBindingDescription{
          // texcoord
          .location = 2,
          .binding = 0,
          .format = Fwog::Format::R32G32_FLOAT,
          .offset = offsetof(Primitives::Vertex, uv),
      },
  };

  auto inputDescs = sceneInputBindingDescs;
  auto primDescs =
      Fwog::InputAssemblyState{Fwog::PrimitiveTopology::TRIANGLE_LIST};

  auto vertexShader =
      Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER,
                   ProjectApplication::LoadFile(vert_indexed_shader_path));
  auto fragmentShader =
      Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER,
                   ProjectApplication::LoadFile(frag_phong_shader_path));

  return Fwog::GraphicsPipeline{{
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .depthState = {.depthTestEnable = true,
                     .depthWriteEnable = true,
                     .depthCompareOp = Fwog::CompareOp::LESS},
  }};
}



void DrawCall::SetBuffers(Fwog::Buffer const& vertex_buffer, Fwog::Buffer const& index_buffer)
{
    this->vertex_buffer = &vertex_buffer;
    this->index_buffer = &index_buffer;
}

void DrawCall::SetModelTransformation(glm::mat4 const& model)
{
    uniform.model = model;
    object_buffer.SubData(uniform, 0);
}

void DrawCall::SetColor(glm::vec4 color)
{
    uniform.color = color;
    object_buffer.SubData(uniform, 0);
}

void DrawCall::Draw(uint64_t stride,uint32_t index_buffer_index, uint32_t object_buffer_index) const
{
    Fwog::Cmd::BindUniformBuffer(object_buffer_index, object_buffer);
    Fwog::Cmd::BindVertexBuffer(index_buffer_index, *vertex_buffer, 0, stride);
    Fwog::Cmd::BindIndexBuffer(*index_buffer, Fwog::IndexType::UNSIGNED_INT);
    Fwog::Cmd::DrawIndexed(static_cast<uint32_t>(index_buffer->Size()) / sizeof(uint32_t) ,1, 0, 0, 0);
}


Fwog::GraphicsPipeline ProjectApplication::CreatePipelineSkybox() {
  static constexpr auto sceneInputBindingDescs =
      std::array{Fwog::VertexInputBindingDescription{
          // position
          .location = 0,
          .binding = 0,
          .format = Fwog::Format::R32G32B32_FLOAT,
          .offset = 0}};

  auto inputDescs = sceneInputBindingDescs;
  auto primDescs =
      Fwog::InputAssemblyState{Fwog::PrimitiveTopology::TRIANGLE_LIST};

  auto vertexShader =
      Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER,
                   ProjectApplication::LoadFile(vert_skybox_shader_path));
  auto fragmentShader =
      Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER,
                   ProjectApplication::LoadFile(frag_skybox_shader_path));

  return Fwog::GraphicsPipeline{{
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .depthState = {.depthTestEnable = true,
                     .depthWriteEnable = true,
                     .depthCompareOp = Fwog::CompareOp::LESS_OR_EQUAL},
  }};
}

void ProjectApplication::AddDebugDrawLine(glm::vec3 ptA, glm::vec3 ptB,
                                          glm::vec3 color) {
  std::array<glm::vec3, 2> linePos{ptA, ptB};
  std::array<glm::vec3, 2> colorPos{color, color};
  vertex_buffer_draw_lines.value().SubData(
      linePos, sizeof(glm::vec3) * curr_num_draw_points);
  vertex_buffer_draw_colors.value().SubData(
      colorPos, sizeof(glm::vec3) * curr_num_draw_points);

  curr_num_draw_points += 2;
}

void ProjectApplication::ClearLines() { curr_num_draw_points = 0; }

void ProjectApplication::DrawLineAABB(Collision::AABB const& aabb,
                                      glm::vec3 boxColor) {
  // It is ok to recalculate face points from the extents despite performance
  // cost because this draw function is optional

  // back : negative z-axis
  // forward : positive z-axis
  glm::vec3 backface_down_left = glm::vec3(aabb.center.x - aabb.halfExtents.x,
                                           aabb.center.y - aabb.halfExtents.y,
                                           aabb.center.z - aabb.halfExtents.z);
  glm::vec3 backface_down_right = glm::vec3(aabb.center.x + aabb.halfExtents.x,
                                            aabb.center.y - aabb.halfExtents.y,
                                            aabb.center.z - aabb.halfExtents.z);
  glm::vec3 backface_up_left = glm::vec3(aabb.center.x - aabb.halfExtents.x,
                                         aabb.center.y + aabb.halfExtents.y,
                                         aabb.center.z - aabb.halfExtents.z);
  glm::vec3 backface_up_right = glm::vec3(aabb.center.x + aabb.halfExtents.x,
                                          aabb.center.y + aabb.halfExtents.y,
                                          aabb.center.z - aabb.halfExtents.z);

  glm::vec3 frontface_down_left = glm::vec3(aabb.center.x - aabb.halfExtents.x,
                                            aabb.center.y - aabb.halfExtents.y,
                                            aabb.center.z + aabb.halfExtents.z);
  glm::vec3 frontface_down_right = glm::vec3(
      aabb.center.x + aabb.halfExtents.x, aabb.center.y - aabb.halfExtents.y,
      aabb.center.z + aabb.halfExtents.z);
  glm::vec3 frontface_up_left = glm::vec3(aabb.center.x - aabb.halfExtents.x,
                                          aabb.center.y + aabb.halfExtents.y,
                                          aabb.center.z + aabb.halfExtents.z);
  glm::vec3 frontface_up_right = glm::vec3(aabb.center.x + aabb.halfExtents.x,
                                           aabb.center.y + aabb.halfExtents.y,
                                           aabb.center.z + aabb.halfExtents.z);

  // Back face
  AddDebugDrawLine(backface_down_left, backface_down_right, boxColor);
  AddDebugDrawLine(backface_down_left, backface_up_left, boxColor);
  AddDebugDrawLine(backface_up_left, backface_up_right, boxColor);
  AddDebugDrawLine(backface_up_right, backface_down_right, boxColor);

  // Front Face
  AddDebugDrawLine(frontface_down_left, frontface_down_right, boxColor);
  AddDebugDrawLine(frontface_down_left, frontface_up_left, boxColor);
  AddDebugDrawLine(frontface_up_left, frontface_up_right, boxColor);
  AddDebugDrawLine(frontface_up_right, frontface_down_right, boxColor);

  // Left Face
  AddDebugDrawLine(backface_down_left, frontface_down_left, boxColor);
  AddDebugDrawLine(backface_up_left, frontface_up_left, boxColor);

  // Right Face
  AddDebugDrawLine(backface_down_right, frontface_down_right, boxColor);
  AddDebugDrawLine(backface_up_right, frontface_up_right, boxColor);
}

void ProjectApplication::DrawLineSphere(Collision::Sphere const& sphere,
                                        glm::vec3 sphereColor) {
  // http://www.songho.ca/opengl/gl_sphere.html

  constexpr uint32_t num_stacks = 16;
  constexpr uint32_t num_slices = 16;

  static glm::vec3 temp_horizontal{0.f};
  static glm::vec3 temp_vert{0.f};
  static glm::vec3 normal{0.f};

  glm::vec3 position_horizontal = sphere.center;
  glm::vec3 position_vert = sphere.center;
  glm::vec3 local_origin = sphere.center;

  float radius = sphere.radius;

  for (uint32_t curr_stack{0}; curr_stack <= num_stacks; ++curr_stack) {
    float theta = static_cast<float>(curr_stack * PI / num_stacks);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);

    for (uint32_t curr_slice{0}; curr_slice <= num_slices; ++curr_slice) {
      float phi = static_cast<float>(curr_slice * 2 * PI / num_slices);
      float sin_phi = sin(phi);
      float cos_phi = cos(phi);

      normal.x = cos_phi * sin_theta;
      normal.y = cos_theta;
      normal.z = sin_phi * sin_theta;

      temp_horizontal = position_horizontal;
      temp_vert = position_vert;

      position_horizontal.x = radius * normal.x;
      position_horizontal.y = radius * normal.y;
      position_horizontal.z = radius * normal.z;

      position_horizontal = local_origin + position_horizontal;

      position_vert.y = radius * normal.x;
      position_vert.x = radius * normal.y;
      position_vert.z = radius * normal.z;

      position_vert = local_origin + position_vert;

      AddDebugDrawLine(position_horizontal, temp_horizontal, sphereColor);
      AddDebugDrawLine(position_vert, temp_vert, sphereColor);
    }
  }
}

void ProjectApplication::AfterCreatedUiContext() {}

void ProjectApplication::BeforeDestroyUiContext() {}

void ProjectApplication::LoadCheckpoints() {
  std::vector<glm::mat4> list =
      Utility::LoadTransformsFromFile("data/levels/checkpoint_layout.gltf");

  for (auto const& transform : list) {
    AddCheckpoint(glm::vec3(0.0f, 0.0f, 0.0f), transform,
                  glm::vec3(1.0f, 1.0f, 1.0f));
  }

  /*AddCheckpoint(glm::vec3(20.0f, 70.0f, 200.0f), glm::mat4{1.0f},
  glm::vec3(1.25f, 1.25f, 1.25f)); AddCheckpoint(glm::vec3(20.0f, 70.0f,
  400.0f), glm::mat4{1.0f}, glm::vec3(1.25f, 1.25f, 1.25f));
  AddCheckpoint(glm::vec3(20.0f, 70.0f, 600.0f),  glm::mat4{1.0f},
  glm::vec3(1.25f, 1.25f, 1.25f));*/
}

void ProjectApplication::AddCheckpoint(glm::vec3 position, glm::mat4 transform,
                                       glm::vec3 scale, float pitch_degrees,
                                       float yaw_degrees) {
  // The Fwog buffers don't have copy constructors so we using emplace to avoid
  // copy construction

  // checkpointList.emplace_back();
  // checkpointObject& checkpoint = checkpointList.back();
  checkpointObject checkpoint;

  checkpoint.center = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
  checkpoint.scale = scale;
  if (checkpointList.empty()) {
    checkpoint.activated = true;
    checkpoint.color = checkpointObject::activated_color_linear;
  }

  checkpoint.collider.center =
      glm::vec3(transform[3].x, transform[3].y, transform[3].z);
  checkpoint.collider.radius = checkpointObject::base_radius * scale.x;

  checkpoint.model = transform;

  // checkpoint.rotation_model_matrix =
  // glm::rotate(checkpoint.rotation_model_matrix, glm::radians(pitch_degrees),
  // worldRight); checkpoint.rotation_model_matrix =
  // glm::rotate(checkpoint.rotation_model_matrix, glm::radians(yaw_degrees),
  // worldUp);

  ObjectUniforms uniform;
  uniform.model = transform;
  uniform.color = glm::vec4(checkpoint.color, 1.0f);

  checkpoint.object_buffer = Fwog::TypedBuffer<ObjectUniforms>(
      Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
  checkpoint.object_buffer.value().SubData(uniform, 0);

  checkpointList.push_back(std::move(checkpoint));
}

void ProjectApplication::CreateSkybox() {
  // So I don't forgor:
  // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#uploading-cube-maps

  using namespace Fwog;

  int32_t textureWidth, textureHeight, textureChannels;
  constexpr int32_t expected_num_channels = 4;

  unsigned char* textureData_skybox_front =
      stbi_load("data/skybox/front.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_front);

  unsigned char* textureData_skybox_back =
      stbi_load("data/skybox/back.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_back);

  unsigned char* textureData_skybox_left =
      stbi_load("data/skybox/left.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_left);

  unsigned char* textureData_skybox_right =
      stbi_load("data/skybox/right.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_right);

  unsigned char* textureData_skybox_up =
      stbi_load("data/skybox/up.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_up);

  unsigned char* textureData_skybox_down =
      stbi_load("data/skybox/down.png", &textureWidth, &textureHeight,
                &textureChannels, expected_num_channels);
  assert(textureData_skybox_down);

  // https://www.khronos.org/opengl/wiki/Cubemap_Texture
  const uint32_t right_id = 0;
  const uint32_t left_id = 1;
  const uint32_t up_id = 2;
  const uint32_t down_id = 3;

  // front instead of forwrad to match the cubemap naming
  const uint32_t front_id = 4;
  const uint32_t back_id = 5;

  uint32_t num_cube_faces = 6;

  Fwog::TextureCreateInfo createInfo{
      .imageType = ImageType::TEX_CUBEMAP,
      .format = Fwog::Format::R8G8B8A8_SRGB,
      .extent = {static_cast<uint32_t>(textureWidth),
                 static_cast<uint32_t>(textureHeight)},
      .mipLevels = uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))),
      .arrayLayers = 1,
      .sampleCount = SampleCount::SAMPLES_1,
  };

  skybox_texture = Fwog::Texture(createInfo);

  // groundAlbedo = Fwog::CreateTexture2DMip({ Fwog::ImageType::TEX_CUBEMAP,
  // static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight)
  // }, Fwog::Format::R8G8B8A8_SRGB, uint32_t(1 +
  // floor(log2(glm::max(textureWidth, textureHeight)))));

  auto upload_face = [&](uint32_t curr_face,
                         unsigned char* texture_pixel_data) {
    Fwog::TextureUpdateInfo updateInfo{
        .dimension = Fwog::UploadDimension::THREE,
        .level = 0,
        .offset = {.depth = curr_face},
        .size = {static_cast<uint32_t>(textureWidth),
                 static_cast<uint32_t>(textureHeight), 1},
        .format = Fwog::UploadFormat::RGBA,
        .type = Fwog::UploadType::UBYTE,
        .pixels = texture_pixel_data};
    skybox_texture.value().SubImage(updateInfo);

    stbi_image_free(texture_pixel_data);
  };

  upload_face(right_id, textureData_skybox_right);
  upload_face(left_id, textureData_skybox_left);
  upload_face(up_id, textureData_skybox_up);
  upload_face(down_id, textureData_skybox_down);
  upload_face(front_id, textureData_skybox_front);
  upload_face(back_id, textureData_skybox_back);

  skybox_texture.value().GenMipmaps();
  pipeline_skybox = CreatePipelineSkybox();
  vertex_buffer_skybox.emplace(Primitives::skybox_vertices);

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void ProjectApplication::LoadGroundPlane() {
  // to do: better texture loading systems. this can break so easily and its
  // jank as hell
  
    int32_t textureWidth, textureHeight, textureChannels;
  constexpr int32_t expected_num_channels = 4;
  
  unsigned char* textureData =
      stbi_load("data/textures/GroundForest003_Flat.png", &textureWidth,
                &textureHeight, &textureChannels, expected_num_channels);
  assert(textureData);


  groundAlbedo = Fwog::CreateTexture2DMip(
      {static_cast<uint32_t>(textureWidth),
       static_cast<uint32_t>(textureHeight)},
      Fwog::Format::R8G8B8A8_SRGB,
      uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))));
  
  Fwog::TextureUpdateInfo updateInfo{
      .dimension = Fwog::UploadDimension::TWO,
      .level = 0,
      .offset = {},
      .size = {static_cast<uint32_t>(textureWidth),
               static_cast<uint32_t>(textureHeight), 1},
      .format = Fwog::UploadFormat::RGBA,
      .type = Fwog::UploadType::UBYTE,
      .pixels = textureData};
  
  groundAlbedo.value().SubImage(updateInfo);
  groundAlbedo.value().GenMipmaps();
  stbi_image_free(textureData);

  glm::mat4 modelPlane = glm::mat4(1.0f);
  modelPlane = glm::scale(modelPlane, planeScale);
  ObjectUniforms planeUniform;
  planeUniform.model = modelPlane;
  objectBufferPlane = Fwog::TypedBuffer<ObjectUniforms>(
      Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
  objectBufferPlane.value().SubData(planeUniform, 0);

  vertex_buffer_plane.emplace(Primitives::plane_vertices);
  index_buffer_plane.emplace(Primitives::plane_indices);
}

void ProjectApplication::LoadBuffers() {
  // Creating world axis stuff
  {
    glm::vec3 worldUpFinal = worldOrigin + (worldUp)*axisScale;
    glm::vec3 worldForwardFinal = worldOrigin + (worldForward)*axisScale;
    glm::vec3 worldRightFinal = worldOrigin + (worldRight)*axisScale;

    std::array<glm::vec3, num_points_world_axis> axisPos{
        worldOrigin,       worldUpFinal, worldOrigin,
        worldForwardFinal, worldOrigin,  worldRightFinal};
    std::array<glm::vec3, num_points_world_axis> axisColors{
        worldUpColor,      worldUpColor,    worldForwardColor,
        worldForwardColor, worldRightcolor, worldRightcolor};

    vertex_buffer_pos_line = Fwog::TypedBuffer<glm::vec3>(axisPos);
    vertex_buffer_color_line = Fwog::TypedBuffer<glm::vec3>(axisColors);
  }

  // Create collision line buffer
  {
    // Doesn't matter what the default initalization is since we only draw the
    // valid points
    std::array<glm::vec3, max_num_draw_points> linePts{};

    vertex_buffer_draw_lines = Fwog::TypedBuffer<glm::vec3>(
        max_num_draw_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    vertex_buffer_draw_colors = Fwog::TypedBuffer<glm::vec3>(
        max_num_draw_points, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    vertex_buffer_draw_lines.value().SubData(linePts, 0);
    vertex_buffer_draw_colors.value().SubData(linePts, 0);
  }

  // Camera Settings
  {
    static glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
    static glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
    static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    static glm::mat4 view = glm::lookAt(camPos, origin, up);
    static glm::mat4 proj =
        glm::perspective(PI / 2.0f, 1.6f, nearPlane, farPlane);
    static glm::mat4 viewProj = proj * view;

    globalStruct.viewProj = viewProj;
    globalStruct.eyePos = camPos;

    globalUniformsBuffer = Fwog::TypedBuffer<GlobalUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    globalUniformsBuffer_skybox = Fwog::TypedBuffer<GlobalUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    globalUniformsBuffer.value().SubData(globalStruct, 0);
    globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);
  }

  // Creating ground plane
  LoadGroundPlane();

  // Creating the aircraft
  {
    Utility::LoadModelFromFile(scene_aircraft,
                               "data/assets/AircraftPropeller.glb",
                               glm::mat4{1.0f}, true);
    ObjectUniforms aircraftUniform;
    aircraftUniform.model = glm::mat4(1.0f);
    aircraftUniform.model = glm::translate(aircraftUniform.model, aircraftPos);
    aircraftUniform.model = glm::scale(aircraftUniform.model, aircraftScale);

    aircraft_box_collider.center = aircraftPos;
    aircraft_box_collider.halfExtents = aircraftScale * aircraftCollisionScale;

    aircraft_sphere_collider.radius = aircraft_sphere_radius;
    aircraft_sphere_collider.center = aircraftPos;

    aircraftUniform.color = aircraftColor;
    objectBufferaircraft = Fwog::TypedBuffer<ObjectUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    object_buffer_propeller = Fwog::TypedBuffer<ObjectUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    objectBufferaircraft.value().SubData(aircraftUniform, 0);

    aircraftUniform.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    object_buffer_propeller.value().SubData(aircraftUniform, 0);
  }

  // Load the actual scene vertices here
  Utility::LoadModelFromFile(scene_collectable,
                             "data/assets/collectableSphere.glb",
                             glm::mat4{1.0f}, true);
  collectableObjectBuffers = Fwog::TypedBuffer<ObjectUniforms>(
      max_num_collectables, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

  building_vertex_buffer.emplace(Primitives::cube_vertices);
  building_index_buffer.emplace(Primitives::cube_indices);

  Utility::LoadModelFromFile(scene_checkpoint_ring,
                             "data/assets/checkpointRing.glb", glm::mat4{1.0f},
                             true);

  // checkpoint_vertex_buffer.emplace(scene_checkpoint_ring.meshes[0].vertexBuffer);
  // checkpoint_index_buffer.emplace(scene_checkpoint_ring.meshes[0].indexBuffer);
}

void ProjectApplication::CreateGroundChunks() {
  // Makes the chunks surrounding relative to a center
  constexpr glm::vec3 center_position{0.0f, 0.0f, 0.0f};
  constexpr glm::vec3 center_chunk_scale{planeScale};

  // length of the world unit vectors (can also be dot product) allows not
  // having to worry about what direction is forward/front
  glm::vec3 forward_chunk_offset{
      center_position.x, center_position.y,
      center_position.z + glm::length(worldForward) * center_chunk_scale.z};
  glm::vec3 back_chunk_offset{
      center_position.x, center_position.y,
      center_position.z - glm::length(worldForward) * center_chunk_scale.z};
  glm::vec3 right_chunk_offset{
      center_position.x + glm::length(worldRight) * center_chunk_scale.x,
      center_position.y, center_position.z};
  glm::vec3 left_chunk_offset{
      center_position.x - glm::length(worldRight) * center_chunk_scale.x,
      center_position.y, center_position.z};

  glm::vec3 forward_right_chunk_offset{
      center_position.x + glm::length(worldRight) * center_chunk_scale.x,
      center_position.y,
      center_position.z + glm::length(worldForward) * center_chunk_scale.z};
  glm::vec3 forward_left_chunk_offset{
      center_position.x - glm::length(worldRight) * center_chunk_scale.x,
      center_position.y,
      center_position.z + glm::length(worldForward) * center_chunk_scale.z};

  glm::vec3 back_right_chunk_offset{
      center_position.x + glm::length(worldRight) * center_chunk_scale.x,
      center_position.y,
      center_position.z - glm::length(worldForward) * center_chunk_scale.z};
  glm::vec3 back_left_chunk_offset{
      center_position.x - glm::length(worldRight) * center_chunk_scale.x,
      center_position.y,
      center_position.z - glm::length(worldForward) * center_chunk_scale.z};

  // I heard rumors of these things called 'constructors'...
  // Not as clumsy or random as a lambda; an elegant weapon for a more civilized
  // age...
  auto create_chunk = [&](glm::vec3 chunk_center) {
    grond_chunk_list.emplace_back();
    ground_chunk& chunk = grond_chunk_list.back();
    chunk.ground_center = chunk_center;
    glm::mat4 model(1.0f);
    model = glm::translate(model, chunk_center);
    model = glm::scale(model, planeScale);
    chunk.ground_uniform.model = model;
    chunk.object_buffer = Fwog::TypedBuffer<ObjectUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    chunk.object_buffer.value().SubData(chunk.ground_uniform, 0);
  };

  create_chunk(forward_chunk_offset);
  create_chunk(back_chunk_offset);
  create_chunk(right_chunk_offset);
  create_chunk(left_chunk_offset);
  create_chunk(forward_right_chunk_offset);
  create_chunk(forward_left_chunk_offset);
  create_chunk(back_right_chunk_offset);
  create_chunk(back_left_chunk_offset);
}

void ProjectApplication::AddCollectable(glm::vec3 position, glm::vec3 scale,
                                        glm::vec3 color) {
  ObjectUniforms collectableUniform;
  collectableUniform.model = glm::mat4(1.0f);
  collectableUniform.model = glm::translate(collectableUniform.model, position);
  collectableUniform.model = glm::scale(collectableUniform.model, scale);
  collectableUniform.color = glm::vec4(color, 1.0f);

  collectableObjectBuffers.value().SubData(
      collectableUniform, sizeof(collectableUniform) * collectableList.size());
  collectableList.emplace_back(position, scale, false);
}

void ProjectApplication::LoadCollectables() {
  // Add placeholder collectables
  constexpr float start_forward_pos = 300.0f;
  constexpr float start_up_pos = 120.0f;
  constexpr float forward_distance_offset = 30.0f;
  constexpr float up_distance_offset = 25.0f;

  constexpr size_t num_collectables = 10;
  // uniform scaling so...
  constexpr float collectable_scale = 4.0f;
  for (size_t i = 0; i < num_collectables; ++i) {
    AddCollectable(
        glm::vec3(0.0f, start_up_pos + up_distance_offset * i,
                  start_forward_pos + i * forward_distance_offset),
        glm::vec3(collectable_scale, collectable_scale, collectable_scale));
  }
}

void ProjectApplication::LoadBuildings() {
  // Do Not export rotations or this will not work as intended. Only scale and
  // translation! This is meant to be AABBs
  std::vector<glm::mat4> transformList = Utility::LoadTransformsFromFile(
      "data/levels/building_collider_layout.gltf");

  for (auto const& transform : transformList) {
    buildingObject object;

    // Just the starting building idea
    glm::mat4 model(1.0f);
    model = transform;

    object.building_center =
        glm::vec3(transform[3].x, transform[3].y, transform[3].z);
    object.building_scale =
        glm::vec3(transform[0].x, transform[1].y, transform[2].z);

    object.building_collider.center = object.building_center;
    object.building_collider.halfExtents = object.building_scale * 0.5f;

    object.drawcall.SetModelTransformation(model);
    object.drawcall.SetColor(glm::vec4{default_building_color, 1.0f});
    object.drawcall.SetBuffers(building_vertex_buffer.value(), building_index_buffer.value());

    buildingObjectList.push_back(std::move(object));
  }
}

void ProjectApplication::SetBackgroundMusic(ma_sound& bgm)
{
    //I hate that this is a global lets fix it in the refactor
    if (curr_background_music_ptr != nullptr)
    {
        ma_sound_stop(curr_background_music_ptr);
    }

    curr_background_music_ptr = &bgm;
    if (!is_background_music_muted) {
        ma_sound_start(curr_background_music_ptr);
    }
}

void ProjectApplication::MuteBackgroundMusicToggle(bool set_muted) {
  is_background_music_muted = set_muted;
  if (curr_game_state == game_states::level_editor)
    SetBackgroundMusic(level_editor_music_ma);
  else {
    SetBackgroundMusic(background_music_ma);
  }
}

bool ProjectApplication::Load() {
  
    configInstance.ParseConfigFile("data/Config.ini");
    std::string windowTitle;
    if (!configInstance.GetDataString("WindowTitle", windowTitle))
        windowTitle = "Missing Config File";

    SetWindowTitle(windowTitle.c_str());

    std::string buffer;
    if (configInstance.GetDataString("MaxSpeed", buffer))
    {
        aircraft_max_speed = std::stof(buffer);
    }
    if (configInstance.GetDataString("MinSpeed", buffer))
    {
        aircraft_min_speed = std::stof(buffer);
    }

    //iniitalize miniaudio
    ma_result ma_res;
    ma_res = ma_engine_init(NULL, &miniAudioEngine);
    if (ma_res != MA_SUCCESS) {
        return -1;
    }

    //ma_engine_play_sound(&miniAudioEngine, "data/sounds/start.wav", NULL);

  // Initialize SoLoud (automatic back-end selection)


  ma_res = ma_sound_init_from_file(&miniAudioEngine, "data/sounds/start.wav", 0, NULL, NULL, &plane_crash_sfx_ma);
  if (ma_res != MA_SUCCESS) {
      return ma_res;
  }
  ma_sound_set_volume(&plane_crash_sfx_ma, 0.75);

  
  ma_res = ma_sound_init_from_file(&miniAudioEngine, "data/sounds/planeflying.wav", 0, NULL, NULL, &plane_flying_sfx_ma);
  if (ma_res != MA_SUCCESS) {
      return ma_res;
  }
  ma_sound_set_volume(&plane_flying_sfx_ma, 0.40);
  ma_sound_set_looping(&plane_flying_sfx_ma, true);
  ma_sound_start(&plane_flying_sfx_ma);

  //res = plane_flying_sfx.load("data/sounds/planeFlying.wav");
  //plane_speedup_sfx.load("data/sounds/planeFlying.wav");

  ma_res = ma_sound_init_from_file(&miniAudioEngine, "data/sounds/backgroundMusic.wav", 0, NULL, NULL, &background_music_ma);
  if (ma_res != MA_SUCCESS) {
      return ma_res;
  }
  ma_sound_set_volume(&background_music_ma, 0.30);

  //background_music.load("data/sounds/backgroundMusic.wav");
  //background_music.setVolume(0.30);

  ma_res = ma_sound_init_from_file(&miniAudioEngine, "data/sounds/levelEditorMusic.wav", 0, NULL, NULL, &level_editor_music_ma);
  if (ma_res != MA_SUCCESS) {
      return ma_res;
  }
  ma_sound_set_volume(&level_editor_music_ma, 0.70);

  
  ma_res = ma_sound_init_from_file(&miniAudioEngine, "data/sounds/collectablePlaceholderSound.wav", 0, NULL, NULL, &plane_collectable_pickup_sfx_ma);
  if (ma_res != MA_SUCCESS) {
      return ma_res;
  }

  ma_sound_set_volume(&plane_crash_sfx_ma, 0.75);

  //Initalized camera

  mainCamera = Camera();

  // Creating pipelines

  pipeline_flat = CreatePipeline();
  pipeline_lines = CreatePipelineLines();
  pipeline_textured = CreatePipelineTextured();
  pipeline_colored_indexed = CreatePipelineColoredIndex();

  LoadBuffers();
  CreateGroundChunks();
  CreateSkybox();

  // LoadCollectables();
  // LoadBuildings();

  // Play sfx


  SetBackgroundMusic(background_music_ma);

  // soloud.play(background_music);

  StartLevel();

  medal_times.insert({gold_level_time, "Gold"});
  medal_times.insert({silver_level_time, "Silver"});
  medal_times.insert({bronze_level_time, "Bronze"});



  return true;
}

void ProjectApplication::ResetLevel() {
  collectableList.clear();
  buildingObjectList.clear();
  checkpointList.clear();

  StartLevel();
}

void ProjectApplication::StartLevel() {

  //When a sound is stopped, it is not rewound to the start -> https://miniaud.io/docs/manual/index.html
  ma_sound_seek_to_pcm_frame(&plane_crash_sfx_ma, 0);
  ma_sound_stop(&plane_crash_sfx_ma);
  ma_sound_start(&plane_flying_sfx_ma);

  LoadBuildings();

  // LoadCollectables();

  LoadCheckpoints();
  curr_active_checkpoint = 0;
  all_checkpoints_collected = false;

  render_plane = true;
  aircraftPos = aircarftStartPos;
  aircraft_body =
      PhysicsBody{aircraft_starting_speed, aircraft_starting_direction_vector};

  ObjectUniforms aircraftUniform;
  aircraftUniform.model = glm::mat4(1.0f);
  aircraftUniform.model = glm::translate(aircraftUniform.model, aircraftPos);
  aircraftUniform.model = glm::scale(aircraftUniform.model, aircraftScale);

  aircraft_box_collider.center = aircraftPos;
  aircraft_box_collider.halfExtents = aircraftScale * aircraftCollisionScale;

  aircraft_sphere_collider.radius = aircraft_sphere_radius;
  aircraft_sphere_collider.center = aircraftPos;

  aircraftUniform.color = aircraftColor;
  objectBufferaircraft.value().SubData(aircraftUniform, 0);

  SetMouseCursorHidden(true);
  current_player_level_time = 0.0f;
}

// Free roam camera controls
void ProjectApplication::UpdateEditorCamera(double dt) {
  // First person camera controls
  static bool first_person_camera_mode = true;
  static bool first_person_button_down = false;

  static constexpr float editor_camera_default_speed_scale = 30.0f;
  static float editor_camera_speed_scale = editor_camera_default_speed_scale;
  editor_camera_speed_scale = editor_camera_default_speed_scale;

  if (IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
    editor_camera_speed_scale = editor_camera_default_speed_scale * 5.0f;
  }

  if (!first_person_button_down && IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
    first_person_camera_mode = !first_person_camera_mode;
    first_person_button_down = true;

    editorCamera.forward =
        glm::normalize(editorCamera.target - editorCamera.position);
    editorCamera.right = glm::cross(editorCamera.forward, worldUp);
    editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);

  } else if (first_person_button_down && IsKeyRelease(GLFW_KEY_GRAVE_ACCENT)) {
    first_person_button_down = false;
  }

  if (first_person_camera_mode) {
    if (IsKeyPressed(GLFW_KEY_W)) {
      editorCamera.position = editorCamera.position +
                              editorCamera.forward * static_cast<float>(dt) *
                                  editor_camera_speed_scale;
      editorCamera.target = editorCamera.target + editorCamera.forward *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsKeyPressed(GLFW_KEY_S)) {
      editorCamera.position = editorCamera.position -
                              editorCamera.forward * static_cast<float>(dt) *
                                  editor_camera_speed_scale;
      editorCamera.target = editorCamera.target - editorCamera.forward *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsKeyPressed(GLFW_KEY_A)) {
      editorCamera.position =
          editorCamera.position - editorCamera.right * static_cast<float>(dt) *
                                      editor_camera_speed_scale;
      editorCamera.target = editorCamera.target - editorCamera.right *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsKeyPressed(GLFW_KEY_D)) {
      editorCamera.position =
          editorCamera.position + editorCamera.right * static_cast<float>(dt) *
                                      editor_camera_speed_scale;
      editorCamera.target = editorCamera.target + editorCamera.right *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsKeyPressed(GLFW_KEY_Q)) {
      editorCamera.position =
          editorCamera.position +
          editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
      editorCamera.target = editorCamera.target + editorCamera.up *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsKeyPressed(GLFW_KEY_E)) {
      editorCamera.position =
          editorCamera.position -
          editorCamera.up * static_cast<float>(dt) * editor_camera_speed_scale;
      editorCamera.target = editorCamera.target - editorCamera.up *
                                                      static_cast<float>(dt) *
                                                      editor_camera_speed_scale;
    }

    if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_1)) {
      MouseRaycast(editorCamera);
    }

    // To Do: Make this cneter of the screen
    static double mouseX = windowWidth / 2;
    static double mouseY = windowHeight / 2;
    static double lastX = windowWidth / 2;
    static double lastY = windowHeight / 2;
    GetMousePosition(mouseX, mouseY);

    static float yawDegrees = 90.0f;
    static float pitchDegrees = 0.0f;

    static bool firstMouseRotate = true;

    if (!(mouseX == std::numeric_limits<double>::infinity() ||
          mouseX == -std::numeric_limits<double>::infinity() ||
          mouseY == std::numeric_limits<double>::infinity() ||
          mouseY == -std::numeric_limits<double>::infinity())) {
      if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_2))  // Right click
      {
        SetMouseCursorHidden(true);

        // Prototype from
        // https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.3.camera_mouse_zoom/camera_mouse_zoom.cpp
        // first

        // Relative to entire window for now

        if (firstMouseRotate) {
          lastX = mouseX;
          lastY = mouseY;
          firstMouseRotate = false;
        }

        float xoffset = mouseX - lastX;
        float yoffset = mouseY - lastY;
        lastX = mouseX;
        lastY = mouseY;

        float sensitivity = 1.0f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yawDegrees += xoffset;
        pitchDegrees += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get
        // flipped
        if (pitchDegrees > 89.0f) pitchDegrees = 89.0f;
        if (pitchDegrees < -89.0f) pitchDegrees = -89.0f;

        editorCamera.forward.x =
            cos(glm::radians(yawDegrees)) * cos(glm::radians(pitchDegrees));
        editorCamera.forward.y = sin(glm::radians(pitchDegrees));
        editorCamera.forward.z =
            sin(glm::radians(yawDegrees)) * cos(glm::radians(pitchDegrees));
        editorCamera.forward = glm::normalize(editorCamera.forward);

        editorCamera.right = glm::cross(editorCamera.forward, worldUp);
        editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);

        editorCamera.target = editorCamera.position + editorCamera.forward;
      } else {
        SetMouseCursorHidden(false);
        firstMouseRotate = true;
      }
    }
  }
}

float ProjectApplication::lerp(float start, float end, float t) {
  return start + (end - start) * t;
}

void ProjectApplication::Update(double dt) {
  static bool gameover_button_down = false;
  if (!gameover_button_down && IsKeyPressed(GLFW_KEY_F)) {
    gameover_button_down = true;
    if (curr_game_state == game_states::playing)
      curr_game_state = game_states::game_over;
    else if (curr_game_state == game_states::game_over)
      curr_game_state = game_states::playing;
  } else if (gameover_button_down && IsKeyRelease(GLFW_KEY_F)) {
    gameover_button_down = false;
  }

  // Change of state
  if (prev_game_state != curr_game_state) {
    if (curr_game_state == game_states::game_over) {
      ma_sound_start(&plane_crash_sfx_ma);
      ma_sound_stop(&plane_flying_sfx_ma);

      render_plane = false;
    } else if (curr_game_state == game_states::playing) {
      if (prev_game_state == game_states::game_over) {
        ResetLevel();
      }
    } else if (curr_game_state == game_states::level_editor) {
      ma_sound_stop(&plane_flying_sfx_ma);
      SetBackgroundMusic(level_editor_music_ma);

      editorCamera = gameplayCamera;

      editorCamera.forward =
          glm::normalize(editorCamera.target - editorCamera.position);
      editorCamera.right = glm::cross(editorCamera.forward, worldUp);
      editorCamera.up = glm::cross(editorCamera.right, editorCamera.forward);
    }

    prev_game_state = curr_game_state;
  }

  if (IsKeyPressed(GLFW_KEY_ESCAPE)) {
    Close();
  }

  //Prototyping reloading the config data
  if (IsKeyPressed(GLFW_KEY_R))
  {

      //ma_engine_uninit(&engine);

      configInstance.ReloadConfigFile("data/Config.ini");

      std::string buffer;
      if (configInstance.GetDataString("MaxSpeed", buffer))
      {
          aircraft_max_speed = std::stof(buffer);
      }
      if (configInstance.GetDataString("MinSpeed", buffer))
      {
          aircraft_min_speed = std::stof(buffer);
      }
      if (configInstance.GetDataString("IncreaseSpeed", buffer))
      {
          aircraft_speed_increase_per_second = std::stof(buffer);
      }

      if (configInstance.GetDataString("DecreaseSpeed", buffer))
      {
          aircraft_speed_decrease_per_second = std::stof(buffer);
      }

  }

  static bool wasKeyPressed_Editor = false;
  if (!wasKeyPressed_Editor && IsKeyPressed(GLFW_KEY_1)) {
    wasKeyPressed_Editor = true;
    curr_game_state = game_states::level_editor;
    SetMouseCursorHidden(false);
  } else if (wasKeyPressed_Editor && IsKeyRelease(GLFW_KEY_1)) {
    wasKeyPressed_Editor = false;
  }

  if (curr_game_state == game_states::level_editor) {
    UpdateEditorCamera(dt);

    // Camera logic stuff
    ZoneScopedC(tracy::Color::Blue);

    glm::mat4 view = glm::lookAt(editorCamera.position, editorCamera.target,
                                 editorCamera.up);
    glm::mat4 proj =
        glm::perspective((base_fov_radians), 1.6f, nearPlane, farPlane);
    glm::mat4 viewProj = proj * view;

    globalStruct.viewProj = viewProj;
    globalStruct.eyePos = editorCamera.position;

    globalUniformsBuffer.value().SubData(globalStruct, 0);

    glm::mat4 view_rot_only = glm::mat4(glm::mat3(view));
    globalStruct.viewProj = proj * view_rot_only;
    globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);

    static bool wasKeyPressed_Gameplay = false;
    if (!wasKeyPressed_Editor && IsKeyPressed(GLFW_KEY_2)) {
      wasKeyPressed_Editor = true;
      curr_game_state = game_states::playing;
      SetBackgroundMusic(background_music_ma);

      ma_sound_start(&plane_flying_sfx_ma);


      SetMouseCursorHidden(true);
    } else if (wasKeyPressed_Editor && IsKeyRelease(GLFW_KEY_2)) {
      wasKeyPressed_Editor = false;
    }
  }

  if (curr_game_state == game_states::playing) {
    {
      // aircraft Inputs
      float dt_float = static_cast<float>(dt);
      float zoom_speed_level =
          min_zoom_level_scale +
          (max_zoom_level_scale - min_zoom_level_scale) *
              (aircraft_body.current_speed / aircraft_max_speed);

      aircraft_current_speed_scale = 1.0f;
      {
        ZoneScopedC(tracy::Color::Green);

        aircraft_body.propeller_angle_degrees =
            lerp(0.0f, 360.0f, elasped_propeller_time);
        elasped_propeller_time +=
            propeller_revolutions_per_second *
            (aircraft_body.current_speed / aircraft_max_speed) * dt;
        if (aircraft_body.propeller_angle_degrees > 360.0f) {
          aircraft_body.propeller_angle_degrees =
              fmod(aircraft_body.propeller_angle_degrees, 360.0f);
          elasped_propeller_time -= 1.0f;
        }

        // aircraft_body.propeller_angle_degrees +=
        // propeller_angle_turning_degrees * dt;

        // To Do: Reset if after 360?

        aircraft_body.forward_vector = worldForward;
        aircraft_body.right_vector = worldRight;
        aircraft_body.up_vector = worldUp;

        aircraft_body.forward_vector =
            glm::vec3(aircraft_body.rotMatrix *
                      glm::vec4(aircraft_body.forward_vector, 1.0f));
        aircraft_body.right_vector =
            glm::vec3(aircraft_body.rotMatrix *
                      glm::vec4(aircraft_body.right_vector, 1.0f));
        aircraft_body.up_vector = glm::vec3(
            aircraft_body.rotMatrix * glm::vec4(aircraft_body.up_vector, 1.0f));


        ma_sound_set_volume(&plane_flying_sfx_ma, 1 * (aircraft_body.current_speed / aircraft_max_speed));
       /* soloud.setVolume(
            plane_flying_sfx_handle,
            1.0 * (aircraft_body.current_speed / aircraft_max_speed));*/

        if (IsKeyPressed(GLFW_KEY_SPACE)) {
          aircraft_current_speed_scale = aircraft_speedup_scale;
          zoom_speed_level = 1.02f;

          //soloud.setVolume(plane_flying_sfx_handle, 0.80);
        }

        // Increase/Lower speed
        if (IsKeyPressed(GLFW_KEY_W) &&
            aircraft_body.current_speed < aircraft_max_speed) {
          aircraft_body.current_speed +=
              aircraft_speed_increase_per_second * dt;
        } else if (IsKeyPressed(GLFW_KEY_S) &&
                   aircraft_body.current_speed > aircraft_min_speed) {
          aircraft_body.current_speed +=
              aircraft_speed_decrease_per_second * dt;
        }

        // Turning Left: Need to adjust both Roll and Velocity
        if (IsKeyPressed(GLFW_KEY_RIGHT)) {
          // aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix,
          // aircraft_angle_turning_degrees * dt_float,
          // aircraft_body.forward_vector);

          // aircraft_body.aircraft_angles_degrees.z +=
          // aircraft_angle_turning_degrees * dt_float;

          // aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix,
          // glm::radians(aircraft_angle_turning_degrees) * dt_float,
          // aircraft_body.forward_vector);
          aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(aircraft_angle_turning_degrees) * dt_float,
              worldForward);
        }

        if (IsKeyPressed(GLFW_KEY_LEFT)) {
          // aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix,
          // glm::radians(-aircraft_angle_turning_degrees) * dt_float,
          // aircraft_body.forward_vector);

          aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(-aircraft_angle_turning_degrees) * dt_float,
              glm::vec3(0.0f, 0.0f, 1.0f));
        }

        if (IsKeyPressed(GLFW_KEY_UP)) {
          // aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix,
          // glm::radians(aircraft_angle_turning_degrees) * dt_float,
          // aircraft_body.right_vector);
          
            
            aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(aircraft_angle_turning_degrees) * dt_float,
              worldRight);



          // aircraft_body.aircraft_angles_degrees.x +=
          // aircraft_angle_turning_degrees * dt_float;
        }

        if (IsKeyPressed(GLFW_KEY_DOWN)) {
          // aircraft_body.rotMatrix = glm::rotate(aircraft_body.rotMatrix,
          // glm::radians(-aircraft_angle_turning_degrees) * dt_float,
          // aircraft_body.right_vector);
          aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(-aircraft_angle_turning_degrees) * dt_float,
              worldRight);

          // aircraft_body.aircraft_angles_degrees.x -=
          // aircraft_angle_turning_degrees * dt_float;
        }

        // Rudders (The yaw)
        if (IsKeyPressed(GLFW_KEY_Q)) {
          aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(aircraft_angle_turning_degrees) * dt_float, worldUp);
        }

        if (IsKeyPressed(GLFW_KEY_E)) {
          aircraft_body.rotMatrix = glm::rotate(
              aircraft_body.rotMatrix,
              glm::radians(-aircraft_angle_turning_degrees) * dt_float,
              worldUp);
        }

        // Update position based off the velocity

        // Experimenting with another approach
        aircraftPos += aircraft_body.forward_vector *
                       aircraft_body.current_speed *
                       aircraft_current_speed_scale * dt_float;

        Collision::SyncSphere(aircraft_sphere_collider, aircraftPos);

        if (draw_player_colliders) {
          DrawLineSphere(aircraft_sphere_collider, glm::vec3(1.0f, 0.0, 0.0f));
        }
      }

      {
        // aircraft uniform buffer changes
        ZoneScopedC(tracy::Color::Orange);
        glm::mat4 model(1.0f);
        glm::mat4 propModel(1.0f);

        model = glm::translate(model, aircraftPos);
        propModel = glm::translate(propModel, aircraftPos);
        propModel = glm::rotate(
            propModel, glm::radians(aircraft_body.propeller_angle_degrees),
            aircraft_body.forward_vector);

        propModel *= aircraft_body.rotMatrix;
        model *= aircraft_body.rotMatrix;

        ObjectUniforms a(model, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
        objectBufferaircraft.value().SubData(a, 0);

        a.model = propModel;
        a.color = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

        object_buffer_propeller.value().SubData(a, 0);
      }

      {
        // Camera logic stuff
        ZoneScopedC(tracy::Color::Blue);

        gameplayCamera.position =
            (aircraftPos - aircraft_body.forward_vector * 25.0f) +
            aircraft_body.up_vector * 10.0f;
        gameplayCamera.target = aircraftPos + (aircraft_body.up_vector * 10.0f);
        gameplayCamera.up = aircraft_body.up_vector;

        glm::mat4 view = glm::lookAt(gameplayCamera.position,
                                     gameplayCamera.target, gameplayCamera.up);
        glm::mat4 view_rot_only = glm::mat4(glm::mat3(view));

        // we dont actually have to recalculate this every frame yet but we
        // might wanna adjust fov i guess
        glm::mat4 proj = glm::perspective((base_fov_radians)*zoom_speed_level,
                                          1.6f, nearPlane, farPlane);
        glm::mat4 viewProj = proj * view;

        globalStruct.viewProj = proj * view_rot_only;
        globalUniformsBuffer_skybox.value().SubData(globalStruct, 0);

        globalStruct.viewProj = viewProj;
        globalStruct.eyePos = gameplayCamera.position;

        globalUniformsBuffer.value().SubData(globalStruct, 0);
      }
    }

    if (!all_checkpoints_collected) current_player_level_time += dt;

    // Collision Checks with collectable
    for (size_t i = 0; i < collectableList.size(); ++i) {
      auto& collectable = collectableList[i];

      if (collectable.isCollected) {
        continue;
      }

      if (draw_collectable_colliders) {
        DrawLineSphere(collectable.collider, glm::vec3(1.0f, 0.0, 0.0f));
      }

      if (Collision::sphereCollisionCheck(aircraft_sphere_collider,
                                          collectable.collider)) {
        
        ma_sound_seek_to_pcm_frame(&plane_collectable_pickup_sfx_ma, 0);
        ma_sound_start(&plane_collectable_pickup_sfx_ma);
        collectable.isCollected = true;

        // Because we use instancing. Decided to simply change the scale to set
        // it to not render. Maybe there is a better way?
        ObjectUniforms temp;
        temp.model = glm::scale(temp.model, glm::vec3(0.0f, 0.0f, 0.0f));
        collectableObjectBuffers.value().SubData(temp,
                                                 sizeof(ObjectUniforms) * i);
      }
    }

    // Collision check with checkpoint (only need to check the next active one!)
    if (!all_checkpoints_collected && !checkpointList.empty() &&
        Collision::sphereCollisionCheck(
            checkpointList[curr_active_checkpoint].collider,
            aircraft_sphere_collider)) {
      checkpointList[curr_active_checkpoint].color =
          checkpointObject::non_activated_color_linear;
      checkpointList[curr_active_checkpoint].activated = false;

      ma_sound_seek_to_pcm_frame(&plane_collectable_pickup_sfx_ma, 0);
      ma_sound_start(&plane_collectable_pickup_sfx_ma);

      if (curr_active_checkpoint + 1 != checkpointList.size()) {
        curr_active_checkpoint += 1;
        checkpointList[curr_active_checkpoint].color =
            checkpointObject::activated_color_linear;

        ObjectUniforms uniform;
        uniform.model = checkpointList[curr_active_checkpoint].model;
        uniform.color =
            glm::vec4(checkpointList[curr_active_checkpoint].color, 1.0f);
        checkpointList[curr_active_checkpoint].object_buffer.value().SubData(
            uniform, 0);
        // checkpointList[curr_active_checkpoint].activated = true;
      } else {
        all_checkpoints_collected = true;
        std::cout
            << "I guess you did it you got all the checkpoints congrats!\n";

        if (personal_best > current_player_level_time) {
          personal_best = current_player_level_time;
          // Get the player's medal state
          for (auto const& x : medal_times) {
            if (current_player_level_time < x.first) {
              player_medal_name = x.second;
              player_medal_timing = x.first;

              // Its already sorted from least time to most time
              break;
            }
          }
        }
      }
    }

    // Collision checks with buildings
    for (auto const& building : buildingObjectList) {
      if (Collision::SphereAABBCollisionCheck(aircraft_sphere_collider,
                                              building.building_collider)) {
        curr_game_state = game_states::game_over;
        break;
      }
    }

    // Check if crashed with the ground
    if (aircraftPos.y < 0.0f) {
      curr_game_state = game_states::game_over;
    }
  }
}

void ProjectApplication::MouseRaycast(camera const& cam) {
  ZoneScopedC(tracy::Color::Purple2);

  glm::mat4 proj = glm::perspective((PI / 2.0f), 1.6f, nearPlane, farPlane);
  glm::mat4 view = glm::lookAt(cam.position, cam.target, cam.up);
  // https://antongerdelan.net/opengl/raycasting.html

  // Viewport Coordinate
  static double mouseX = windowWidth / 2;
  static double mouseY = windowHeight / 2;
  GetMousePosition(mouseX, mouseY);

  // NDC Coordinate
  float x = (2.0f * mouseX) / windowWidth - 1.0f;
  float y = 1.0f - (2.0f * mouseY) / windowHeight;
  float z = 1.0f;
  glm::vec3 ray_nds = glm::vec3(x, y, z);

  // Homogenous Clip Coordinates
  glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

  // View Space Coordinates
  glm::vec4 ray_eye = inverse(proj) * ray_clip;

  ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

  // World Coordinates
  glm::vec4 ray_wor = (inverse(view) * ray_eye);
  glm::vec3 ray_world_vec3 = glm::vec3(ray_wor.x, ray_wor.y, ray_wor.z);
  ray_world_vec3 = glm::normalize(ray_world_vec3);

  constexpr float debug_mouse_click_length = 5000.0f;

  // std::vector<buildingObject*> accepted_colliders;

  // for (auto& buillding : buildingObjectList)
  //{
  //	//To Do: Rewrite the raycast logic (it should check every collider it
  //passes for every step so it can do early rejections) 	if
  //(Collision::RaycastCheck(cam.position, ray_world_vec3,
  //buillding.building_collider))
  //	{
  //		accepted_colliders.push_back(&buillding);
  //		break;
  //	}
  // }

  // if (accepted_colliders.empty())
  //	return;

  // std::sort<buildingObject*>(accepted_colliders.front(),
  // accepted_colliders.back(), [&](auto const& lhs, auto const& rhs)
  //{
  //		glm::vec3 lhs_vec = (lhs.building_center - cam.position);
  //		glm::vec3 rhs_vec = (rhs.building_center - cam.position);
  //		float dist_lhs = glm::dot(lhs_vec, lhs_vec);
  //		float dist_rhs = glm::dot(rhs_vec, rhs_vec);
  //		return dist_lhs < dist_rhs;
  // });

  buildingObject* building_hit =
      RaycastCheck(cam.position, ray_world_vec3, buildingObjectList);
  if (building_hit == nullptr) return;


  building_hit->drawcall.SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 0));

  // Yea we should create a function for this
  // 
  // 
  //ObjectUniforms temp;
  //glm::mat4 model(1.0f);
  //model = glm::translate(model, building_hit->building_center);
  //model = glm::scale(model, building_hit->building_scale);
  //temp.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  //temp.model = model;
  //building_hit->object_buffer.value().SubData(temp, 0);

  // AddDebugDrawLine(cam.position, cam.position + ray_world_vec3 *
  // debug_mouse_click_length, glm::vec3(0.0f, 0.0f, 1.0f));
}

void ProjectApplication::RenderScene() {
  // RenderMousePick();

  ZoneScopedC(tracy::Color::Red);

  Fwog::BeginSwapchainRendering(Fwog::SwapchainRenderInfo{
      .viewport =
          Fwog::Viewport{.drawRect{.offset = {0, 0},
                                   .extent = {windowWidth, windowHeight}},
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f},
      .colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
      .clearColorValue = {skyColorFoggy.r, skyColorFoggy.g, skyColorFoggy.b,
                          1.0f},
      .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
      .clearDepthValue = 1.0f});

  // Drawing a ground plane
  {
    Fwog::SamplerState ss;
    ss.minFilter = Fwog::Filter::LINEAR;
    ss.magFilter = Fwog::Filter::LINEAR;
    ss.mipmapFilter = Fwog::Filter::LINEAR;
    ss.addressModeU = Fwog::AddressMode::REPEAT;
    ss.addressModeV = Fwog::AddressMode::REPEAT;
    ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
    auto nearestSampler = Fwog::Sampler(ss);

    Fwog::Cmd::BindGraphicsPipeline(pipeline_textured.value());
    Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
    Fwog::Cmd::BindUniformBuffer(1, objectBufferPlane.value());
    Fwog::Cmd::BindSampledImage(0, groundAlbedo.value(), nearestSampler);
    Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_plane.value(), 0,
                                sizeof(Primitives::Vertex));
    Fwog::Cmd::BindIndexBuffer(index_buffer_plane.value(),
                               Fwog::IndexType::UNSIGNED_INT);
    Fwog::Cmd::DrawIndexed(
        static_cast<uint32_t>(Primitives::plane_indices.size()), 1, 0, 0, 0);

    // Drawing the other ground planes
    for (auto const& ground : grond_chunk_list) {
      Fwog::Cmd::BindUniformBuffer(1, ground.object_buffer.value());
      Fwog::Cmd::BindSampledImage(0, groundAlbedo.value(), nearestSampler);
      Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_plane.value(), 0,
                                  sizeof(Primitives::Vertex));
      Fwog::Cmd::BindIndexBuffer(index_buffer_plane.value(),
                                 Fwog::IndexType::UNSIGNED_INT);
      Fwog::Cmd::DrawIndexed(
          static_cast<uint32_t>(Primitives::plane_indices.size()), 1, 0, 0, 0);
    }
  }

  // Drawing buildings
  {
    static constexpr uint64_t stride = sizeof(Utility::Vertex);
    for (auto const& building : buildingObjectList) {
      Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
      Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
      building.drawcall.Draw(stride);
    }
  }

  // Drawing the collectables
  {
    if (!collectableList.empty()) {
      Fwog::Cmd::BindGraphicsPipeline(pipeline_colored_indexed.value());
      Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
      Fwog::Cmd::BindStorageBuffer(1, collectableObjectBuffers.value());
      Fwog::Cmd::BindVertexBuffer(0, scene_collectable.meshes[0].vertexBuffer,
                                  0, sizeof(Utility::Vertex));
      Fwog::Cmd::BindIndexBuffer(scene_collectable.meshes[0].indexBuffer,
                                 Fwog::IndexType::UNSIGNED_INT);
      Fwog::Cmd::DrawIndexed(
          static_cast<uint32_t>(
              scene_collectable.meshes[0].indexBuffer.Size()) /
              sizeof(uint32_t),
          collectableList.size(), 0, 0, 0);
    }
  }

  // Drawing checkpoints
  {
    // Assumptions: All checkpoints are allocated in collection sequence
    // linearly.
    if (!all_checkpoints_collected) {
      for (size_t i = curr_active_checkpoint; i < checkpointList.size(); ++i) {
        Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
        Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
        Fwog::Cmd::BindUniformBuffer(1,
                                     checkpointList[i].object_buffer.value());
        Fwog::Cmd::BindVertexBuffer(
            0, scene_checkpoint_ring.meshes[0].vertexBuffer, 0,
            sizeof(Primitives::Vertex));
        Fwog::Cmd::BindIndexBuffer(scene_checkpoint_ring.meshes[0].indexBuffer,
                                   Fwog::IndexType::UNSIGNED_INT);
        Fwog::Cmd::DrawIndexed(
            static_cast<uint32_t>(
                scene_checkpoint_ring.meshes[0].indexBuffer.Size()) /
                sizeof(uint32_t),
            1, 0, 0, 0);
      }
    }
  }

  // Drawing a aircraft
  if (render_plane) {
    Fwog::Cmd::BindGraphicsPipeline(pipeline_flat.value());
    Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
    Fwog::Cmd::BindUniformBuffer(1, objectBufferaircraft.value());
    Fwog::Cmd::BindVertexBuffer(0, scene_aircraft.meshes[1].vertexBuffer, 0,
                                sizeof(Utility::Vertex));
    Fwog::Cmd::BindIndexBuffer(scene_aircraft.meshes[1].indexBuffer,
                               Fwog::IndexType::UNSIGNED_INT);
    Fwog::Cmd::DrawIndexed(
        static_cast<uint32_t>(scene_aircraft.meshes[1].indexBuffer.Size()) /
            sizeof(uint32_t),
        1, 0, 0, 0);

    Fwog::Cmd::BindUniformBuffer(1, object_buffer_propeller.value());
    Fwog::Cmd::BindVertexBuffer(0, scene_aircraft.meshes[0].vertexBuffer, 0,
                                sizeof(Utility::Vertex));
    Fwog::Cmd::BindIndexBuffer(scene_aircraft.meshes[0].indexBuffer,
                               Fwog::IndexType::UNSIGNED_INT);
    Fwog::Cmd::DrawIndexed(
        static_cast<uint32_t>(scene_aircraft.meshes[0].indexBuffer.Size()) /
            sizeof(uint32_t),
        1, 0, 0, 0);
  }

  // Drawing axis lines
  {
    Fwog::Cmd::BindGraphicsPipeline(pipeline_lines.value());
    Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
    if (renderAxis) {
      Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_pos_line.value(), 0,
                                  3 * sizeof(float));
      Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_color_line.value(), 0,
                                  3 * sizeof(float));
      Fwog::Cmd::Draw(num_points_world_axis, 1, 0, 0);
    }

    // Drawing collision lines
    if (curr_num_draw_points != 0 &&
        curr_num_draw_points < max_num_draw_points) {
      Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_draw_lines.value(), 0,
                                  3 * sizeof(float));
      Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_draw_colors.value(), 0,
                                  3 * sizeof(float));
      Fwog::Cmd::Draw(curr_num_draw_points, 1, 0, 0);

      // Allows DrawLine to be called every frame without creating buffers.
      // Should make new buffer  if want presistent lines ofc
      ClearLines();
    }
  }

  // Drawing skybox last depth buffer
  {
    Fwog::SamplerState ss;
    ss.minFilter = Fwog::Filter::LINEAR;
    ss.magFilter = Fwog::Filter::LINEAR;
    ss.mipmapFilter = Fwog::Filter::LINEAR;
    ss.addressModeU = Fwog::AddressMode::REPEAT;
    ss.addressModeV = Fwog::AddressMode::REPEAT;
    ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
    auto nearestSampler = Fwog::Sampler(ss);

    Fwog::Cmd::BindGraphicsPipeline(pipeline_skybox.value());
    Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer_skybox.value());
    Fwog::Cmd::BindSampledImage(0, skybox_texture.value(), nearestSampler);
    Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_skybox.value(), 0,
                                3 * sizeof(float));
    Fwog::Cmd::Draw(Primitives::skybox_vertices.size() / 3, 1, 0, 0);
  }

  Fwog::EndRendering();
}

void ProjectApplication::RenderUI(double dt) {
  // This is needed or else there's a crash
  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

  ImGui::Begin("Performance");
  {
    ImGui::Text("Framerate: %.0f Hertz", 1 / dt);
    ImGui::End();
  }

  ImGui::Begin("How To Play");
  {
    ImGui::Text("Use the arrow keys to pitch and roll!");
    ImGui::Text("Use Q/E to control your rudder (Yaw)");
    ImGui::Text("Use W/S to control your speed");
    ImGui::Text("Press 'F' to Restart");
    ImGui::Text("Press '1' to pause and '2' to unpause");
    ImGui::End();
  }

  ImGui::Begin("Options (Pause to Release Mouse)");
  {
    static bool set_background_music = is_background_music_muted;
    ImGui::Checkbox("Background Music Muted?", &set_background_music);
    if (set_background_music != is_background_music_muted) {
      is_background_music_muted = set_background_music;
      MuteBackgroundMusicToggle(is_background_music_muted);
    }
    ImGui::End();
  }

  ImGui::Begin("Gameplay");
  {
    ImGui::Text("Checkpoint: %d/%d",
                curr_active_checkpoint +
                    static_cast<uint32_t>(all_checkpoints_collected),
                checkpointList.size());
    ImGui::Text("Current Time: %f", current_player_level_time);
    if (all_checkpoints_collected) {
      ImGui::Text("Level Completed");
      ImGui::Text("Your personal best is: %f", personal_best);
      ImGui::Text("You medal is %s which has a requirement of %f",
                  player_medal_name.c_str(), player_medal_timing);
    }

    ImGui::End();
  }
}

ProjectApplication::~ProjectApplication() {
  ma_engine_uninit(&miniAudioEngine);
}

}  // namespace PlaneGame
