#include <SandboxApplication.hpp>

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

#include <Albuquerque/Primitives.hpp>

static constexpr float PI = 3.1415926f;


SandboxApplication::Pipeline::Pipeline(std::string_view vertex_shader_path, std::string_view fragment_shader_path)
{
    auto LoadFile = [](std::string_view path)
    {
        std::ifstream file{ path.data() };
        std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        return returnString;
    };

    auto LoadShader = [&LoadFile](GLuint shader_id, std::string_view path)
    {
        //Prevent dangling pointer by moving it to local variable
        std::string returnString = std::move(LoadFile(path));
        const GLchar* shader_contents = returnString.data();
        glShaderSource(shader_id, 1, &shader_contents, nullptr);
        glCompileShader(shader_id);

        //To Do: Check if shader compiled correctly with glGetShaderiv()
    };
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    LoadShader(vertex_shader, vertex_shader_path);
    LoadShader(fragment_shader, fragment_shader_path);
}

SandboxApplication::ShaderProgram::ShaderProgram()
{
    shader_program = glCreateProgram();
    //To Do: Check for errors
}

GLuint SandboxApplication::ShaderProgram::GetShader()
{
    //Read-only... but OpenGL is a global state machine so who cares honestly
    return shader_program; 
}

void SandboxApplication::Pipeline::AttachToShader(ShaderProgram& shader) const
{
    if (vertex_shader != 0)
        glAttachShader(shader.GetShader(), vertex_shader);
    if (fragment_shader != 0)
        glAttachShader(shader.GetShader(), fragment_shader);


    //To Do: Error checking
    glLinkProgram(shader.GetShader());
}

void SandboxApplication::ShaderProgram::BeginDraw()
{
    glUseProgram(shader_program);
}

void SandboxApplication::ShaderProgram::EndDraw()
{
    glUseProgram(0);
}

SandboxApplication::ExampleTriangle::ExampleTriangle()
{

    std::array<float, num_points * 3> positions = {
       -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    GLuint vbo = 0;
    vao = 0;

    glCreateBuffers(1, &vao);
    glNamedBufferData(vao, sizeof(float), positions.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);
    glBindVertexBuffer(0, vbo, 0, sizeof(float) * 3.0);

    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);
}

void SandboxApplication::ExampleTriangle::Draw(ShaderProgram& shaderProgram)
{
    shaderProgram.BeginDraw();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, num_points);
    shaderProgram.EndDraw();
}

void SandboxApplication::AfterCreatedUiContext()
{

}

void SandboxApplication::BeforeDestroyUiContext()
{

}

bool SandboxApplication::Load()
{
    if (!Application::Load())
    {
        spdlog::error("App: Unable to load");
        return false;
    }
    SetWindowTitle("Sandbox");



    return true;
}

void SandboxApplication::Update(double dt)
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        Close();
    }

}

void SandboxApplication::RenderScene(double dt)
{

}

void SandboxApplication::RenderUI(double dt)
{
    ImGui::Begin("Window");
    {
        ImGui::TextUnformatted("Hello World!");
        ImGui::End();
    }

    //ImGui::ShowDemoWindow();
}