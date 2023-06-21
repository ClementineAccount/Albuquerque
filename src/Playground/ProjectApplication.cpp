#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Project/ProjectApplication.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>
#include <queue>
#include <set>

static constexpr float PI = 3.1415926f;

static std::string Slurp(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::ate);
    std::string result(file.tellg(), '\0');
    file.seekg(0);
    file.read((char*)result.data(), result.size());
    return result;
}

namespace fs = std::filesystem;

static std::string FindTexturePath(const fs::path& basePath, const cgltf_image* image)
{
    std::string texturePath;
    if (!image->uri)
    {
        auto newPath = basePath / image->name;
        if (!newPath.has_extension())
        {
            if (std::strcmp(image->mime_type, "image/png") == 0)
            {
                newPath.replace_extension("png");
            }
            else if (std::strcmp(image->mime_type, "image/jpg") == 0)
            {
                newPath.replace_extension("jpg");
            }
        }
        texturePath = newPath.generic_string();
    }
    else
    {
        texturePath = (basePath / image->uri).generic_string();
    }
    return texturePath;
}


template <typename T1, typename T2>
DrawObject DrawObject::Init(T1 const& vertexList, T2 const& indexList, size_t indexCount)
{
    DrawObject object;
    object.vertexBuffer.emplace(vertexList);
    object.indexBuffer.emplace(indexList);
    object.modelUniformBuffer =  Fwog::TypedBuffer<DrawObject::ObjectUniform>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    object.modelUniformBuffer.value().SubData(object.objectStruct, 0);

    //Fwog takes in uint32_t for the indexCount but .size() on a container returns size_t. I'll just cast it here and hope its fine.
    object.indexCount = static_cast<uint32_t>(indexCount);
    
    return object;
}


void Camera::Update()
{

    glm::mat4 view = glm::lookAt(camPos,  target,  up);
    glm::mat4 viewSky = glm::mat4(glm::mat3(view));
    glm::mat4 proj = glm::perspective(PI / 2.0f, 1.6f, nearPlane, farPlane);

    cameraStruct.viewProj = proj * view;
    cameraStruct.eyePos = camPos;
    cameraUniformsBuffer.value().SubData(cameraStruct, 0);

    cameraStruct.viewProj = proj * viewSky;

    cameraUniformsSkyboxBuffer.value().SubData(cameraStruct, 0);
}

Camera::Camera()
{
    nearPlane = 0.01f;
    farPlane = 5000.0f;

    camPos = glm::vec3(3.0f, 3.0f, 3.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f); //Target the origin
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(camPos,  target,  up);
    glm::mat4 proj =
        glm::perspective(PI / 2.0f, 1.6f, nearPlane, farPlane);

    cameraStruct.viewProj = proj * view;
    cameraStruct.eyePos = camPos;
    cameraUniformsBuffer = Fwog::TypedBuffer<Camera::CameraUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

    cameraUniformsSkyboxBuffer = Fwog::TypedBuffer<Camera::CameraUniforms>(
        Fwog::BufferStorageFlag::DYNAMIC_STORAGE);

    cameraUniformsBuffer.value().SubData(cameraStruct, 0);

    Update();
}



Skybox::Skybox()
{
    pipeline = MakePipleine("./data/shaders/skybox.vs.glsl", "./data/shaders/skybox.fs.glsl");
    texture = MakeTexture();
    vertexBuffer.emplace(Primitives::skyboxVertices);
}

Fwog::GraphicsPipeline Skybox::MakePipleine(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
{
    auto LoadFile = [](std::string_view path)
    {
        std::ifstream file{ path.data() };
        std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        return returnString;
    };

    auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, LoadFile(vertexShaderPath));
    auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, LoadFile(fragmentShaderPath));

    static constexpr auto sceneInputBindingDescs =
        std::array{Fwog::VertexInputBindingDescription{
        // position
        .location = 0,
        .binding = 0,
        .format = Fwog::Format::R32G32B32_FLOAT,
        .offset = 0}};

    auto inputDescs = sceneInputBindingDescs;
    auto primDescs = Fwog::InputAssemblyState{Fwog::PrimitiveTopology::TRIANGLE_LIST};

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

Fwog::Texture Skybox::MakeTexture()
{
    using namespace Fwog;

    int32_t textureWidth, textureHeight, textureChannels;
    constexpr int32_t expected_num_channels = 4;

    unsigned char* textureData_skybox_front =
        stbi_load("./data/textures/skybox/front.png", &textureWidth, &textureHeight,
            &textureChannels, expected_num_channels);
    assert(textureData_skybox_front);

    unsigned char* textureData_skybox_back =
        stbi_load("./data/textures/skybox/back.png", &textureWidth, &textureHeight,
            &textureChannels, expected_num_channels);
    assert(textureData_skybox_back);

    unsigned char* textureData_skybox_left =
        stbi_load("./data/textures/skybox/left.png", &textureWidth, &textureHeight,
            &textureChannels, expected_num_channels);
    assert(textureData_skybox_left);

    unsigned char* textureData_skybox_right =
        stbi_load("./data/textures/skybox/right.png", &textureWidth, &textureHeight,
            &textureChannels, expected_num_channels);
    assert(textureData_skybox_right);

    unsigned char* textureData_skybox_up =
        stbi_load("./data/textures/skybox/up.png", &textureWidth, &textureHeight,
            &textureChannels, expected_num_channels);
    assert(textureData_skybox_up);

    unsigned char* textureData_skybox_down =
        stbi_load("./data/textures/skybox/down.png", &textureWidth, &textureHeight,
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
        .mipLevels =
        uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight)))),
        .arrayLayers = 1,
        .sampleCount = SampleCount::SAMPLES_1,
    };

    Fwog::Texture texture = Fwog::Texture(createInfo);


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
            texture.SubImage(updateInfo);

            stbi_image_free(texture_pixel_data);
    };

    upload_face(right_id, textureData_skybox_right);
    upload_face(left_id, textureData_skybox_left);
    upload_face(up_id, textureData_skybox_up);
    upload_face(down_id, textureData_skybox_down);
    upload_face(front_id, textureData_skybox_front);
    upload_face(back_id, textureData_skybox_back);

    texture.GenMipmaps();

    return texture;
}

void GameObject::UpdateDraw()
{
    glm::mat4& model = drawData.objectStruct.modelTransform;
    model = glm::mat4(1.0f);

    //Translate, Scale than Rotate

    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    //tbh we don't need this for the example
    model = glm::rotate(model, glm::radians(eulerAngleDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(eulerAngleDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(eulerAngleDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));

    drawData.modelUniformBuffer.value().SubData(drawData.objectStruct, 0);
}

Fwog::GraphicsPipeline ProjectApplication::MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
{
    auto LoadFile = [](std::string_view path)
    {
        std::ifstream file{ path.data() };
        std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        return returnString;
    };

    auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, LoadFile(vertexShaderPath));
    auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, LoadFile(fragmentShaderPath));

    //Ensures this matches the shader and your vertex buffer data type

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


Fwog::Texture ProjectApplication::MakeTexture(std::string_view texturePath, int32_t expectedChannels)
{
    int32_t textureWidth, textureHeight, textureChannels;
    unsigned char* textureData =
        stbi_load(texturePath.data(), &textureWidth, &textureHeight, &textureChannels, expectedChannels);
    assert(textureData);

    //How many times can this texture be divided evenly by half?
    uint32_t divideByHalfAmounts =  uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight))));

    Fwog::Texture createdTexture = Fwog::CreateTexture2DMip(
        {static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight)},
        Fwog::Format::R8G8B8A8_SRGB,
        divideByHalfAmounts);

    Fwog::TextureUpdateInfo updateInfo{
        .dimension = Fwog::UploadDimension::TWO,
        .level = 0,
        .offset = {},
        .size = {static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight), 1},
        .format = Fwog::UploadFormat::RGBA,
        .type = Fwog::UploadType::UBYTE,
        .pixels = textureData};

    createdTexture.SubImage(updateInfo);
    createdTexture.GenMipmaps();
    stbi_image_free(textureData);

    return createdTexture;
}


void ProjectApplication::AfterCreatedUiContext()
{
}

void ProjectApplication::BeforeDestroyUiContext()
{

}


bool ProjectApplication::Load()
{
    if (!Application::Load())
    {
        spdlog::error("App: Unable to load");
        return false;
    }

    pipelineTextured = MakePipeline("./data/shaders/main.vs.glsl", "./data/shaders/main.fs.glsl");
    for (size_t i = 0; i < numCubes; ++i)
    {
        //https://en.cppreference.com/w/cpp/language/class_template_argument_deduction 
        //because the containers which are the parameters are constexpr
        exampleCubes[i].drawData = DrawObject::Init(Primitives::cubeVertices, Primitives::cubeIndices, Primitives::cubeIndices.size());

        //Offset the transforms of the cube

        static constexpr float offsetForward = 10.0f;
        exampleCubes[i].position.z -= i * offsetForward;

        exampleCubes[i].scale *= (i + 1);

        exampleCubes[i].UpdateDraw();
    }

    cubeTexture = MakeTexture("./data/textures/fwog_logo.png");

    sceneCamera = Camera();


    skybox = Skybox();

    return true;
}

void ProjectApplication::Update(double dt)
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        Close();
    }

    //This is an arcball style update. Could move it maybe?
    auto updateCameraArc = [&](Camera& currCamera)
    {
        bool isUpdate = false;
        static float camSpeed = 2.0f;
        
        if (IsKeyPressed(GLFW_KEY_A))
        {
            isUpdate = true;
            currCamera.camPos.x += camSpeed * dt;
        }
        else if (IsKeyPressed(GLFW_KEY_D))
        {
            isUpdate = true;
            currCamera.camPos.x -= camSpeed * dt;
        }

        if (IsKeyPressed(GLFW_KEY_Q))
        {
            isUpdate = true;
            currCamera.camPos.z += camSpeed * dt;
        }
        else if (IsKeyPressed(GLFW_KEY_E))
        {
            isUpdate = true;
            currCamera.camPos.z -= camSpeed * dt;
        }

        if (IsKeyPressed(GLFW_KEY_W))
        {
            isUpdate = true;
            currCamera.camPos.y += camSpeed * dt;
        }
        else if (IsKeyPressed(GLFW_KEY_S))
        {
            isUpdate = true;
            currCamera.camPos.y -= camSpeed * dt;
        }

        //we only need to recalculate the viewProj if camera data did change
        if (isUpdate)
            currCamera.Update();
    };

    updateCameraArc(sceneCamera.value());
}

void ProjectApplication::RenderScene()
{

    static constexpr glm::vec4 backgroundColor = glm::vec4(0.1f, 0.3f, 0.2f, 1.0f);
    Fwog::BeginSwapchainRendering(Fwog::SwapchainRenderInfo{
        .viewport =
        Fwog::Viewport{.drawRect{.offset = {0, 0},
        .extent = {windowWidth, windowHeight}},
        .minDepth = 0.0f,
        .maxDepth = 1.0f},
        .colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
        .clearColorValue = {backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a},
        .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
        .clearDepthValue = 1.0f});

    Fwog::SamplerState ss;
    ss.minFilter = Fwog::Filter::LINEAR;
    ss.magFilter = Fwog::Filter::LINEAR;
    ss.mipmapFilter = Fwog::Filter::LINEAR;
    ss.addressModeU = Fwog::AddressMode::REPEAT;
    ss.addressModeV = Fwog::AddressMode::REPEAT;
    ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
    static auto nearestSampler = Fwog::Sampler(ss);

    //Could refactor this to be a function of a class
    auto drawObject = [&](DrawObject const& object, Fwog::Texture const& textureAlbedo, Fwog::Sampler const& sampler, Camera const& camera)
    {
        Fwog::Cmd::BindGraphicsPipeline(pipelineTextured.value());
        Fwog::Cmd::BindUniformBuffer(0, camera.cameraUniformsBuffer.value());
        Fwog::Cmd::BindUniformBuffer(1, object.modelUniformBuffer.value());

        Fwog::Cmd::BindSampledImage(0, textureAlbedo, sampler);
        Fwog::Cmd::BindVertexBuffer(0, object.vertexBuffer.value(), 0, sizeof(Primitives::Vertex));
        Fwog::Cmd::BindIndexBuffer(object.indexBuffer.value(), Fwog::IndexType::UNSIGNED_INT);
        Fwog::Cmd::DrawIndexed(object.indexCount, 1, 0, 0, 0);
    };

    for (size_t i = 0; i < numCubes; ++i)
    {
        drawObject(exampleCubes[i].drawData, cubeTexture.value(), nearestSampler, sceneCamera.value());
    }

    auto drawSkybox = [&](Skybox const& skybox, Fwog::Sampler const& sampler, Camera const& camera)
    {
        Fwog::Cmd::BindGraphicsPipeline(skybox.pipeline.value());
        Fwog::Cmd::BindUniformBuffer(0, camera.cameraUniformsSkyboxBuffer.value());

        Fwog::Cmd::BindSampledImage(0, skybox.texture.value(), sampler);
        Fwog::Cmd::BindVertexBuffer(0, skybox.vertexBuffer.value(), 0, 3 * sizeof(float));
        Fwog::Cmd::Draw(Primitives::skyboxVertices.size() / 3, 1, 0, 0);
    };

    if (_skyboxVisible)
        drawSkybox(skybox.value(), nearestSampler, sceneCamera.value());

    Fwog::EndRendering();

}

void ProjectApplication::RenderUI(double dt)
{
    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello Fwog!");
        ImGui::TextUnformatted("Use WASD and QE for Arcball Controls.");
        ImGui::Checkbox("Skybox", &_skyboxVisible);
        ImGui::End();
    }

    //ImGui::ShowDemoWindow();
}




//bool ProjectApplication::MakeShader(std::string_view vertexShaderFilePath, std::string_view fragmentShaderFilePath)
//{
//    int success = false;
//    char log[1024] = {};
//    const auto vertexShaderSource = Slurp(vertexShaderFilePath);
//    const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
//    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
//    glCompileShader(vertexShader);
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(vertexShader, 1024, nullptr, log);
//        spdlog::error(log);
//        return false;
//    }
//
//    const auto fragmentShaderSource = Slurp(fragmentShaderFilePath);
//    const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
//    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
//    glCompileShader(fragmentShader);
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(fragmentShader, 1024, nullptr, log);
//        spdlog::error(log);
//        return false;
//    }
//
//    shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//    if (!success)
//    {
//        glGetProgramInfoLog(shaderProgram, 1024, nullptr, log);
//        spdlog::error(log);
//
//        return false;
//    }
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return true;
//}
//
