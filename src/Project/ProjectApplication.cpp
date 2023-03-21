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

    if (!MakeShader("./data/shaders/main.vs.glsl", "./data/shaders/main.fs.glsl"))
    {
        return false;
    }

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
    const auto projection = glm::perspective(glm::radians(80.0f), 1920.0f / 1080.0f, 0.1f, 256.0f);
    const auto view = glm::lookAt(
        glm::vec3(3 * std::cos(glfwGetTime() / 4), 2, -3 * std::sin(glfwGetTime() / 4)),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shaderProgram);
    glUniformMatrix4fv(0, 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));

    struct ObjectData
    {
        uint32_t transformIndex;
        uint32_t baseColorIndex;
        uint32_t normalIndex;
    };
    struct BatchData
    {
        std::vector<ObjectData> objects;
        std::vector<MeshIndirectInfo> indirectCommands;
    };
    std::vector<BatchData> objectBatches(_cubes.Commands.size());
    std::vector<std::set<uint32_t>> textureHandles(_cubes.Commands.size());
    for (const auto& mesh : _cubes.Meshes)
    {
        const auto index = mesh.BaseColorTexture / 16;
        objectBatches[index].indirectCommands.emplace_back(MeshIndirectInfo
        {
            mesh.IndexCount,
            1,
            mesh.indexOffset,
            mesh.VertexOffset,
            1
        });
        objectBatches[index].objects.emplace_back(ObjectData
        {
            mesh.TransformIndex,
            mesh.BaseColorTexture % 16,
            mesh.NormalTexture
        });
        textureHandles[index].insert(_cubes.Textures[mesh.BaseColorTexture]);
    }

    glNamedBufferData(
        _cubes.TransformData,
        _cubes.Transforms.size() * sizeof(glm::mat4),
        _cubes.Transforms.data(),
        GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _cubes.TransformData);

    for (uint32_t index = 0; const auto& batch : objectBatches)
    {
        glNamedBufferData(
            _cubes.ObjectData[index],
            batch.objects.size() * sizeof(ObjectData),
            batch.objects.data(),
            GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cubes.ObjectData[index]);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _cubes.Commands[index]);
        glNamedBufferData(
            _cubes.Commands[index],
            batch.indirectCommands.size() * sizeof(MeshIndirectInfo),
            batch.indirectCommands.data(),
            GL_DYNAMIC_DRAW);

        for (uint32_t offset = 0; const auto texture : textureHandles[index])
        {
            glUniform1i(2 + offset, offset);
            glActiveTexture(GL_TEXTURE0 + offset);
            glBindTexture(GL_TEXTURE_2D, texture);
            offset++;
        }
        glBindVertexArray(_cubes.InputLayout);
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            batch.indirectCommands.size(),
            sizeof(MeshIndirectInfo));
        index++;
    }
}

void ProjectApplication::RenderUI()
{
    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello World!");
        ImGui::End();
    }

    ImGui::ShowDemoWindow();
}

bool ProjectApplication::MakeShader(std::string_view vertexShaderFilePath, std::string_view fragmentShaderFilePath)
{
    int success = false;
    char log[1024] = {};
    const auto vertexShaderSource = Slurp(vertexShaderFilePath);
    const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 1024, nullptr, log);
        spdlog::error(log);
        return false;
    }

    const auto fragmentShaderSource = Slurp(fragmentShaderFilePath);
    const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 1024, nullptr, log);
        spdlog::error(log);
        return false;
    }

    _shaderProgram = glCreateProgram();
    glAttachShader(_shaderProgram, vertexShader);
    glAttachShader(_shaderProgram, fragmentShader);
    glLinkProgram(_shaderProgram);
    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_shaderProgram, 1024, nullptr, log);
        spdlog::error(log);

        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}