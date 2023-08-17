
#include <PlaygroundApplication.hpp>
#include <Voxel.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <span>
#include <iostream>

#include <Albuquerque/Primitives.hpp>

#include <stb_image.h>

static constexpr float PI = 3.1415926f;


Skybox::Skybox()
{
    pipeline = MakePipleine("./data/shaders/skybox.vs.glsl", "./data/shaders/skybox.fs.glsl");
    texture = MakeTexture();
    vertexBuffer.emplace(Albuquerque::Primitives::skyboxVertices);
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
                .offset = { .z = curr_face },
                    .extent{static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight), 1},
                    .format = Fwog::UploadFormat::RGBA,
                    .type = Fwog::UploadType::UBYTE,
                    .pixels = texture_pixel_data
            };
            texture.UpdateImage(updateInfo);

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

    drawData.modelUniformBuffer.value().UpdateData(drawData.objectStruct, 0);
}

Fwog::GraphicsPipeline PlaygroundApplication::MakePipeline(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
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

    auto sceneInputBindingDescs = std::array{
        Fwog::VertexInputBindingDescription{
            // position
            .location = 0,
            .binding = 0,
            .format = Fwog::Format::R32G32B32_FLOAT,
            .offset = offsetof(Albuquerque::Primitives::Vertex, position),
    },
    Fwog::VertexInputBindingDescription{
            // normal
            .location = 1,
            .binding = 0,
            .format = Fwog::Format::R32G32B32_FLOAT,
            .offset = offsetof(Albuquerque::Primitives::Vertex, normal),
    },
    Fwog::VertexInputBindingDescription{
            // texcoord
            .location = 2,
            .binding = 0,
            .format = Fwog::Format::R32G32_FLOAT,
            .offset = offsetof(Albuquerque::Primitives::Vertex, uv),
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


Fwog::Texture PlaygroundApplication::MakeTexture(std::string_view texturePath, int32_t expectedChannels)
{
    stbi_set_flip_vertically_on_load(true);
    int32_t textureWidth, textureHeight, textureChannels;
    unsigned char* textureData =
        stbi_load(texturePath.data(), &textureWidth, &textureHeight, &textureChannels, expectedChannels);
    assert(textureData);
    stbi_set_flip_vertically_on_load(false);

    //How many times can this texture be divided evenly by half?
    uint32_t divideByHalfAmounts =  uint32_t(1 + floor(log2(glm::max(textureWidth, textureHeight))));

    Fwog::Texture createdTexture = Fwog::CreateTexture2DMip(
        {static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight)},
        Fwog::Format::R8G8B8A8_SRGB,
        divideByHalfAmounts);

    Fwog::TextureUpdateInfo updateInfo{
        .extent = { static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight), 1 },
            .format = Fwog::UploadFormat::RGBA,
            .type = Fwog::UploadType::UBYTE,
            .pixels = textureData};

    createdTexture.UpdateImage(updateInfo);
    createdTexture.GenMipmaps();
    stbi_image_free(textureData);

    return createdTexture;
}



ViewData::ViewData()
{
    viewBuffer = Fwog::TypedBuffer<ViewUniform>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    skyboxBuffer = Fwog::TypedBuffer<ViewUniform>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
}

void ViewData::Update(Albuquerque::Camera const& camera)
{
    static ViewUniform viewUniform;

    glm::mat4 view = glm::lookAt(camera.camPos,  camera.camTarget,  camera.camUp);
    glm::mat4 viewSky = glm::mat4(glm::mat3(view));
    glm::mat4 proj = glm::perspective(PI / 2.0f, 1.6f, camera.nearPlane, camera.farPlane);

    viewUniform.viewProj = proj * view;
    viewUniform.eyePos = camera.camPos;
    viewBuffer.value().UpdateData(viewUniform, 0);

    viewUniform.viewProj = proj * viewSky;

    skyboxBuffer.value().UpdateData(viewUniform, 0);
}

void PlaygroundApplication::AfterCreatedUiContext()
{
}

void PlaygroundApplication::BeforeDestroyUiContext()
{

}

bool PlaygroundApplication::LoadFwog()
{
    using namespace Albuquerque;
    //Create the meshBuffer
    Albuquerque::FwogHelpers::MeshBuffer::GetMap().emplace(std::make_pair("cube", Albuquerque::FwogHelpers::MeshBuffer::Init(Primitives::cubeVertices, Primitives::cubeIndices, Primitives::cubeIndices.size())));


    pipelineTextured_ = MakePipeline("./data/shaders/main.vs.glsl", "./data/shaders/main.fs.glsl");
    for (size_t i = 0; i < numCubes_; ++i)
    {
        using namespace Albuquerque;

        //https://en.cppreference.com/w/cpp/language/class_template_argument_deduction 
        //because the containers which are the parameters are constexpr

        exampleCubes_[i].drawData = Albuquerque::FwogHelpers::DrawObject::Init("cube");

        //Offset the transforms of the cube

        static constexpr float offsetForward = 10.0f;
        exampleCubes_[i].position.z -= i * offsetForward;
        exampleCubes_[i].scale *= (i + 1);
        exampleCubes_[i].eulerAngleDegrees.x = 25.0f;
        exampleCubes_[i].UpdateDraw();
    }

    cubeTexture_ = MakeTexture("./data/textures/whiteFace.png");

    viewData_ = ViewData();
    viewData_->Update(sceneCamera_);

    skybox_ = Skybox();

    voxelGrid_ = VoxelStuff::Grid();

    //It does not
    //std::cout << "Does this go to spdlog?\n";

    line_renderer = LineRendererFwog();

    constexpr float current_axis_length = 100000.0f;

    //Do axis line as example
    auto drawAxis = [&](float axisLength)
    {
        line_renderer->AddPoint(glm::vec3(0.0f, 0.0f, 0.0f));
        line_renderer->AddPoint(glm::vec3(axisLength, 0.0f, 0.0f));

        line_renderer->AddPoint(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        line_renderer->AddPoint(glm::vec3(0.0f, axisLength, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        line_renderer->AddPoint(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        line_renderer->AddPoint(glm::vec3(0.0f, 0.0f, axisLength), glm::vec3(0.0f, 0.0f, 1.0f));
    };

    drawAxis(current_axis_length);





    return true;
}

bool PlaygroundApplication::Load()
{
    if (!Application::Load())
    {
        spdlog::error("App: Unable to load");
        return false;
    }
    SetWindowTitle("Playground");

    return LoadFwog();
}

void PlaygroundApplication::UpdateMouseOffset(double dt, double& xOffset, double& yOffset)
{
    static bool isFirstPressed = true;
    static double xOrigin = 0;
    static double yOrigin = 0;

    //To Do: Decouple the mouse input check
    if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_2))
    {
        //Set mouse center using hidden as recommended by GLFW https://www.glfw.org/docs/3.3/input_guide.html#cursor_set
        SetMouseCursorDisabled(true);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(_windowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

        //spdlog::info("Right click down");
        if (isFirstPressed)
        {
            isFirstPressed = false;
            xOffset = 0;
            yOffset = 0;

            glfwGetCursorPos(_windowHandle, &xOrigin, &yOrigin);
            //spdlog::info("xOrigin: {:03.2f}, yOrigin: {:03.2f}", xOrigin, yOrigin);
        }
        else
        {
            //Track the offset by getting mouse position and calculating it from the origin
            double xpos, ypos;
            glfwGetCursorPos(_windowHandle, &xpos, &ypos);
            xOffset = xpos - xOrigin;
            yOffset = ypos - yOrigin;

            //spdlog::info("xoffset: {:03.2f}, yOffset: {:03.2f}", xOffset, yOffset);
        }
    }
    else
    {
        isFirstPressed = true;
        xOrigin = 0;
        yOrigin = 0;
        xOffset = 0;
        yOffset = 0;

        SetMouseCursorDisabled(false);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(_windowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
}

void PlaygroundApplication::UpdateFwog(double dt)
{
    auto updateCamera = [&](Albuquerque::Camera& currCamera)
    {
        using namespace Albuquerque;

        bool isUpdate = false;
        static float camSpeedBase = 2.0f;
        static float camSpeedMouseBase = 0.5f;
        float camSpeed = camSpeedBase * static_cast<float>(dt);
        float camSpeedMouse = camSpeedMouseBase * static_cast<float>(dt);

        if (IsKeyPressed(GLFW_KEY_A))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveLeft, camSpeed);
        }
        else if (IsKeyPressed(GLFW_KEY_D))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveRight, camSpeed);
        }

        if (IsKeyPressed(GLFW_KEY_Q))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveUp, camSpeed);
        }
        else if (IsKeyPressed(GLFW_KEY_E))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveDown, camSpeed);
        }

        if (IsKeyPressed(GLFW_KEY_W))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveForward, camSpeed);
        }
        else if (IsKeyPressed(GLFW_KEY_S))
        {
            isUpdate = true;
            currCamera.MoveFly(Camera::directionalInput::moveBack, camSpeed);
        }

        //Testing if rotations are working ok

        //Left and Right
        if (IsKeyPressed(GLFW_KEY_H))
        {
            isUpdate = true;
            currCamera.RotateFly(camSpeed * 10.0f, 0.0f);
        }
        else if (IsKeyPressed(GLFW_KEY_F))
        {
            isUpdate = true;
            currCamera.RotateFly(-camSpeed * 10.0f, 0.0f);
        }

        if (IsKeyPressed(GLFW_KEY_T))
        {
            isUpdate = true;
            currCamera.RotateFly(0.0f, camSpeed * 10.0f);
        }
        else if (IsKeyPressed(GLFW_KEY_G))
        {
            isUpdate = true;
            currCamera.RotateFly(0.0f, -camSpeed * 10.0f);
        }
        static double x = 0;
        static double y = 0;
        UpdateMouseOffset(dt, x, y);

        if (x != 0 || y != 0)
        {
            isUpdate = true;
            currCamera.RotateFly(camSpeedMouse * x, -camSpeedMouse * y);
        }


        //we only need to recalculate the viewProj if camera data did change
        if (isUpdate)
            viewData_->Update(currCamera);
    };

    updateCamera(sceneCamera_);

    //Draws a line from origin to raycast point. Acts as a 'test' for both functions
    auto rayCastTest = [&](Albuquerque::Camera const& currCamera, LineRendererFwog& lineRenderer)
    {
        //This will **add** more lines there too... every frame as well which is cursed

        //TODO: Make IsKeyJustPressed check
        static bool wasClicked = false;
        if (IsMouseKeyPressed(GLFW_MOUSE_BUTTON_1) && !wasClicked) {
            lineRenderer.Clear();

            wasClicked = true;
            //TODO: Check if normalization does anything even
            glm::vec3 ray = glm::normalize(RaycastScreenToWorld(currCamera));
            glm::vec3 worldPoint = currCamera.camPos + ray;
            lineRenderer.AddPoint(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
            lineRenderer.AddPoint(worldPoint, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        else if (!IsMouseKeyPressed(GLFW_MOUSE_BUTTON_1) && wasClicked)
        {
            wasClicked = false;
        }
    };

    rayCastTest(sceneCamera_, line_renderer.value());
}

void PlaygroundApplication::Update(double dt)
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        Close();
    }

    if (fwogScene_)
        UpdateFwog(dt);
}

void PlaygroundApplication::RenderFwog(double dt)
{
    static constexpr glm::vec4 backgroundColor = glm::vec4(0.1f, 0.3f, 0.2f, 1.0f);
    Fwog::SamplerState ss;
    ss.minFilter = Fwog::Filter::LINEAR;
    ss.magFilter = Fwog::Filter::LINEAR;
    ss.mipmapFilter = Fwog::Filter::LINEAR;
    ss.addressModeU = Fwog::AddressMode::REPEAT;
    ss.addressModeV = Fwog::AddressMode::REPEAT;
    ss.anisotropy = Fwog::SampleCount::SAMPLES_16;
    static auto nearestSampler = Fwog::Sampler(ss);

    //Could refactor this to be a function of a class
    auto drawObject = [&](Albuquerque::FwogHelpers::DrawObject const& object, Fwog::Texture const& textureAlbedo, Fwog::Sampler const& sampler, ViewData const& viewData)
    {
        Fwog::Cmd::BindGraphicsPipeline(pipelineTextured_.value());
        Fwog::Cmd::BindUniformBuffer(0, viewData.viewBuffer.value());
        Fwog::Cmd::BindUniformBuffer(1, object.modelUniformBuffer.value());

        Fwog::Cmd::BindSampledImage(0, textureAlbedo, sampler);

        //To Do: Add some 'draw mesh buffer'
        auto drawMeshBuffer = [&](Albuquerque::FwogHelpers::MeshBuffer const& meshBuffer)
        {
            Fwog::Cmd::BindVertexBuffer(0, *meshBuffer.vertexBuffer, 0, sizeof(Albuquerque::Primitives::Vertex));
            Fwog::Cmd::BindIndexBuffer(*meshBuffer.indexBuffer, Fwog::IndexType::UNSIGNED_INT);
            Fwog::Cmd::DrawIndexed(meshBuffer.indexCount, 1, 0, 0, 0);
        };

        drawMeshBuffer(*object.meshBufferRef);

        //Fwog::Cmd::BindVertexBuffer(0, object.vertexBuffer.value(), 0, sizeof(Albuquerque::Primitives::Vertex));
        //Fwog::Cmd::BindIndexBuffer(object.indexBuffer.value(), Fwog::IndexType::UNSIGNED_INT);
        //Fwog::Cmd::DrawIndexed(object.indexCount, 1, 0, 0, 0);
    };


    auto drawSkybox = [&](Skybox const& skybox, Fwog::Sampler const& sampler)
    {
        Fwog::Cmd::BindGraphicsPipeline(skybox.pipeline.value());
        Fwog::Cmd::BindUniformBuffer(0, viewData_->skyboxBuffer.value());

        Fwog::Cmd::BindSampledImage(0, skybox.texture.value(), sampler);
        Fwog::Cmd::BindVertexBuffer(0, skybox.vertexBuffer.value(), 0, 3 * sizeof(float));
        Fwog::Cmd::Draw(Albuquerque::Primitives::skyboxVertices.size() / 3, 1, 0, 0);
    };

    Fwog::RenderToSwapchain(
        {
        .viewport =
        Fwog::Viewport{.drawRect{.offset = {0, 0},
        .extent = {windowWidth, windowHeight}},
        .minDepth = 0.0f,
        .maxDepth = 1.0f},
        .colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
        .clearColorValue = {backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a},
        .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
        .clearDepthValue = 1.0f
        },
        [&]
        {
            if (fwogScene_)
            {
                line_renderer->Draw(*viewData_);

                for (size_t i = 0; i < numCubes_; ++i)
                {
                    //drawObject(exampleCubes_[i].drawData, cubeTexture_.value(), nearestSampler, viewData_.value());
                }

                if (skyboxVisible_)
                    drawSkybox(skybox_.value(), nearestSampler);

                voxelGrid_->Draw(cubeTexture_.value(), nearestSampler, viewData_.value());
            }
        }
        );
}

void PlaygroundApplication::RenderScene(double dt)
{
    RenderFwog(dt);
}

void PlaygroundApplication::RenderUI(double dt)
{
    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello World!");
        ImGui::Checkbox("Show Fwog Scene", &fwogScene_);
        if (fwogScene_)
        {
            ImGui::TextUnformatted("Use WASD and QE for Arcball Controls.");
            ImGui::Checkbox("Skybox", &skyboxVisible_);
        }


        ImGui::End();
    }

    ImGui::Begin("Performance");
    {
        ImGui::Text("Framerate: %.0f Hertz", 1 / dt);
        ImGui::End();
    }
    //ImGui::ShowDemoWindow();
}

glm::vec3 PlaygroundApplication::RaycastScreenToWorld(Albuquerque::Camera const& cam)
{
    glm::mat4 proj = glm::perspective((PI / 2.0f), 1.6f, cam.nearPlane, cam.farPlane);
    glm::mat4 view = glm::lookAt(cam.camPos, cam.camTarget, cam.camUp);
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
    //Caller normalizes it, not the callee (as non-normalized gets the world point)
    return ray_world_vec3;
}

LineRendererFwog::LineRendererFwog()
{
    //To Do: Have this passed in as parameters
    constexpr char vertexShaderPath[] = "./data/shaders/lines.vert.glsl";
    constexpr char fragmentShaderPath[] = "./data/shaders/lines.frag.glsl";

    //Create pipeline 
    auto LoadFile = [](std::string_view path)
    {
        std::ifstream file{ path.data() };
        std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        return returnString;
    };

    auto vertex_shader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, LoadFile(vertexShaderPath));
    auto fragment_shader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, LoadFile(fragmentShaderPath));

    static constexpr auto sceneInputBindingDescs = std::array{
    Fwog::VertexInputBindingDescription{
        // position
        .location = 0,
            .binding = 0,
            .format = Fwog::Format::R32G32B32_FLOAT,
            .offset = 0
    },
    Fwog::VertexInputBindingDescription{
        .location = 1,
            .binding = 1,
            .format = Fwog::Format::R32G32B32_FLOAT,
            .offset = 0,
    }};

    auto inputDescs = sceneInputBindingDescs;
    auto primDescs = Fwog::InputAssemblyState{ Fwog::PrimitiveTopology::LINE_LIST };

    pipeline =  Fwog::GraphicsPipeline{{
            .vertexShader = &vertex_shader,
                .fragmentShader = &fragment_shader,
                .inputAssemblyState = primDescs,
                .vertexInputState = { inputDescs },
                .depthState = { .depthTestEnable = true,
                .depthWriteEnable = true,
                .depthCompareOp = Fwog::CompareOp::LESS_OR_EQUAL },
    }};

    //Create buffers
    vertex_buffer = Fwog::TypedBuffer<glm::vec3>(maxPoints, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
    color_buffer = Fwog::TypedBuffer<glm::vec3>(maxPoints, Fwog::BufferStorageFlag::DYNAMIC_STORAGE);
}

void LineRendererFwog::AddPoint(glm::vec3 point_position, glm::vec3 point_color)
{
    vertex_buffer->UpdateData(point_position, point_count);
    color_buffer->UpdateData(point_color, point_count);
    ++point_count;

    //TODO: Add consideration for when point_count > max amount... right now Fwog will crash instead
}

void LineRendererFwog::Draw(ViewData const& viewData)
{
    Fwog::Cmd::BindGraphicsPipeline(pipeline.value());
    Fwog::Cmd::BindUniformBuffer(0, viewData.viewBuffer.value());
    Fwog::Cmd::BindVertexBuffer(0, vertex_buffer.value(), 0, 3 * sizeof(float));
    Fwog::Cmd::BindVertexBuffer(1, color_buffer.value(), 0, 3 * sizeof(float));
    Fwog::Cmd::Draw(point_count, 1, 0, 0);
}

void LineRendererFwog::Clear()
{
    //Not needed. Just keep the old data and just don't pass it during rendering (and new data will override it)
    //vertex_buffer->ClearSubData(Fwog::BufferClearInfo());
    //color_buffer->ClearSubData(Fwog::BufferClearInfo());

    point_count = 0;
}