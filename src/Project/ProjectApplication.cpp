//#define CGLTF_IMPLEMENTATION
//#include <cgltf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "include/MeshLoading.h"
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

//static std::string FindTexturePath(const fs::path& basePath, const cgltf_image* image)
//{
//    std::string texturePath;
//    if (!image->uri)
//    {
//        auto newPath = basePath / image->name;
//        if (!newPath.has_extension())
//        {
//            if (std::strcmp(image->mime_type, "image/png") == 0)
//            {
//                newPath.replace_extension("png");
//            }
//            else if (std::strcmp(image->mime_type, "image/jpg") == 0)
//            {
//                newPath.replace_extension("jpg");
//            }
//        }
//        texturePath = newPath.generic_string();
//    }
//    else
//    {
//        texturePath = (basePath / image->uri).generic_string();
//    }
//    return texturePath;
//}

void ProjectApplication::AfterCreatedUiContext()
{

}

void ProjectApplication::BeforeDestroyUiContext()
{

}


//VAOs for it. We rendering it like a baby
draw_call CreateTriangle()
{
    uint32_t vbo = 0;
    uint32_t vao = 0;

    //anti clockwise
    glm::vec3 tri_pos[3];
    tri_pos[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
    tri_pos[1] = glm::vec3(1.0f, -1.0f, 0.0f);
    tri_pos[2] = glm::vec3(0.0f, 0.5f, 0.0f);

    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(tri_pos), tri_pos, GL_DYNAMIC_STORAGE_BIT);
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayAttribBinding(vao, 0, 0);

    return draw_call{true, vao, 3};
}



draw_call CreateShape(std::vector<glm::vec3> const& positions, std::vector<glm::vec4> const& colors, std::vector<uint32_t> indices)
{
    uint32_t vbo = 0;
    uint32_t vao = 0;
    uint32_t ibo = 0;

    glCreateBuffers(1, &vbo);


    //Really ineffiicent should be passsing in an array/vector of Vertex struct objects instead but lazy rn

    std::vector<float> dataVertex;
    for (auto const& pos : positions)
    {
        dataVertex.push_back(pos.x);
        dataVertex.push_back(pos.y);
        dataVertex.push_back(pos.z);
    }

    for (auto const& col : colors)
    {
        dataVertex.push_back(col.r);
        dataVertex.push_back(col.g);
        dataVertex.push_back(col.b);
        dataVertex.push_back(col.a);
    }


    glNamedBufferStorage(vbo, sizeof(float) * dataVertex.size(), dataVertex.data(), GL_DYNAMIC_STORAGE_BIT);


    glCreateBuffers(1, &ibo);
    glNamedBufferStorage(ibo, sizeof(uint32_t) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 3.0f);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vao, 1, vbo, sizeof(float) * positions.size() * 3, sizeof(float) * 4.0f);
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayElementBuffer(vao, ibo);
    glVertexArrayAttribBinding(vao, 0, 0);

    return draw_call{ false, vao, static_cast<GLsizei>(indices.size())};
}

draw_call CreateSquare()
{
    //anti clockwise, based off ndc
    glm::vec3 square_pos[4];
    
    //bottom left
    square_pos[0] = glm::vec3(-1.0f, -1.0f, 0.0f);

    //bottom right
    square_pos[1] = glm::vec3(1.0f, -1.0f, 0.0f);

    //top right
    square_pos[2] = glm::vec3(1.0f, 1.0f, 0.0f);

    //top left
    square_pos[3] = glm::vec3(-1.0f, 1.0f, 0.0f);


    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    uint32_t vbo = 0;
    uint32_t vao = 0;
    uint32_t ibo = 0;

    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(square_pos), square_pos, GL_DYNAMIC_STORAGE_BIT);


    glCreateBuffers(1, &ibo);
    glNamedBufferStorage(ibo, sizeof(uint32_t) * 6, indices, GL_DYNAMIC_STORAGE_BIT);


    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);


    glVertexArrayElementBuffer(vao, ibo);

    glVertexArrayAttribBinding(vao, 0, 0);




    return draw_call{ false, vao, 6 };


}



void DrawHelloPrim(uint32_t shader_id, draw_call const& draw)
{

    constexpr glm::vec3 eyePos = glm::vec3(10.0f, 10.0f, 10.0f);
    constexpr glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

    constexpr float fov = 90.0f;

    constexpr float nearPlane = 0.01f;
    constexpr float farPlane = 100.0f;

    constexpr float aspect_ratio = 1.6f;

    static glm::mat4 viewMatrix = glm::lookAt(eyePos, origin, glm::vec3(0.0f, 1.0f, 0.0f));
    static glm::mat4 projMatrix = glm::perspective(fov, aspect_ratio, nearPlane, farPlane);


    glUseProgram(shader_id);

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(1, 1, false, glm::value_ptr(projMatrix));


    glBindVertexArray(draw.vao);

    if (draw.drawArrays)
        glDrawArrays(GL_TRIANGLES, 0, draw.numIndices);
    else
        glDrawElements(GL_TRIANGLES, draw.numIndices, GL_UNSIGNED_INT, 0);

    //glDrawArrays(GL_TRIANGLES, 0, 3);
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

    std::vector<glm::vec3> pos;
    std::vector<uint32_t> indices;


    MeshLoader loader;
    Mesh mesh = loader.loadDeccers("./data/models/SM_Deccer_Cubes.gltf");

    MeshLoadingTest::loadGLTF_Basic("./data/models/test_cube.gltf", pos, indices);


    helloPrim = CreateShape(mesh.positions, mesh.colors, mesh.indices);

    //helloPrim = CreateSquare();

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawHelloPrim(_shaderProgram, helloPrim);
}

void ProjectApplication::RenderUI()
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello World!");
        ImGui::End();
    }


    //ImGui::ShowDemoWindow();
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