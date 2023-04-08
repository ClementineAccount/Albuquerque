//#define CGLTF_IMPLEMENTATION
//#include <cgltf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Project/ProjectApplication.hpp>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <spdlog/spdlog.h>

#include <Fwog/BasicTypes.h>
#include <Fwog/Buffer.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>


#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <array>

static std::string Slurp(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::ate);
    std::string result(file.tellg(), '\0');
    file.seekg(0);
    file.read((char*)result.data(), result.size());
    return result;
}

namespace fs = std::filesystem;


static constexpr char vert_shader_path[] = "data/shaders/FwogRacing/hello_car.vert.glsl";
static constexpr char frag_shader_path[] = "data/shaders/FwogRacing/hello_car.frag.glsl";
static constexpr char frag_texture_shader_path[] = "data/shaders/FwogRacing/hello_car_textured.frag.glsl";

static constexpr char vert_line_shader_path[] = "data/shaders/FwogRacing/lines.vert.glsl";
static constexpr char frag_line_shader_path[] = "data/shaders/FwogRacing/lines.frag.glsl";

std::string ProjectApplication::LoadFile(std::string_view path)
{
    std::ifstream file{ path.data() };
    return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}

static Fwog::GraphicsPipeline CreatePipeline()
{
    // Specify our two vertex attributes: position and color.
    // Positions are 3x float, so we will use R32G32B32_FLOAT like we would in Vulkan.
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
    auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::TRIANGLE_LIST };


    auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_shader_path));
    auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_shader_path));

    return Fwog::GraphicsPipeline{ {
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .depthState = {.depthTestEnable = true, .depthWriteEnable = true, .depthCompareOp = Fwog::CompareOp::LESS},
    } };
}


static Fwog::GraphicsPipeline CreatePipelineLines()
{
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

    auto inputDescs = { descPos, descColor };

    auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::LINE_LIST };
    auto depthDescs = Fwog::DepthState{ .depthTestEnable = false, .depthWriteEnable = false };
    auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, ProjectApplication::LoadFile(vert_line_shader_path));
    auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, ProjectApplication::LoadFile(frag_line_shader_path));

    return Fwog::GraphicsPipeline{ {
      .vertexShader = &vertexShader,
      .fragmentShader = &fragmentShader,
      .inputAssemblyState = primDescs,
      .vertexInputState = {inputDescs},
      .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
      .depthState = {.depthTestEnable = false, .depthWriteEnable = false},
    } };
}


void ProjectApplication::AfterCreatedUiContext()
{

}

void ProjectApplication::BeforeDestroyUiContext()
{

}



bool ProjectApplication::Load()
{
    pipeline = CreatePipeline();
    pipeline_lines = CreatePipelineLines();

    glm::vec3 worldUpFinal = worldOrigin + (worldUp)*axisScale;
    glm::vec3 worldForwardFinal = worldOrigin + (worldForward)*axisScale;
    glm::vec3 worldRightFinal = worldOrigin + (worldRight)*axisScale;


    std::array<glm::vec3, num_points_world_axis> axisPos{ worldOrigin, worldUpFinal, worldOrigin, worldForwardFinal, worldOrigin, worldRightFinal };
    std::array<glm::vec3, num_points_world_axis> axisColors{ worldUpColor,
                                        worldUpColor,
                                        worldForwardColor,
                                        worldForwardColor,
                                        worldRightcolor,
                                        worldRightcolor };

    vertex_buffer_pos_line = Fwog::TypedBuffer<glm::vec3>(axisPos);
    vertex_buffer_color_line = Fwog::TypedBuffer<glm::vec3>(axisColors);


    static glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
    static glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
    static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    static glm::mat4 view = glm::lookAt(camPos, origin, up);
    static glm::mat4 proj = glm::perspective(PI / 2.0f, 1.6f, nearPlane, farPlane);

    static glm::mat4 viewProj = proj * view;

    globalUniformsBuffer = Fwog::TypedBuffer<GlobalUniforms>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    globalUniformsBuffer.value().SubData(viewProj, 0);
    
    return true;
}

void ProjectApplication::Update()
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        Close();
    }
}



void ProjectApplication::RenderScene()
{
    Fwog::BeginSwapchainRendering(Fwog::SwapchainRenderInfo{
    .viewport =
      Fwog::Viewport{
        .drawRect{.offset = {0, 0}, .extent = {windowWidth, windowHeight}},
        .minDepth = 0.0f, .maxDepth = 1.0f
      },
    .colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
    .clearColorValue = {skyColor.r, skyColor.g, skyColor.b, 1.0f},
     .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
      .clearDepthValue = 1.0f
        });

    Fwog::Cmd::BindGraphicsPipeline(pipeline_lines.value());
    Fwog::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
    Fwog::Cmd::BindVertexBuffer(0, vertex_buffer_pos_line.value(), 0, 3 * sizeof(float));
    Fwog::Cmd::BindVertexBuffer(1, vertex_buffer_color_line.value(), 0, 3 * sizeof(float));
    Fwog::Cmd::Draw(num_points_world_axis, 1, 0, 0);

    Fwog::EndRendering();
}

void ProjectApplication::RenderUI()
{
    //This is needed or else there's a crash
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello F!");
        ImGui::End();
    }
}
