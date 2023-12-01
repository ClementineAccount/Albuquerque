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

char shaderText[1024];

bool isShaderText = false;

SandboxApplication::Pipeline::Pipeline(std::string_view vertex_shader_path, std::string_view fragment_shader_path)
{
    auto LoadFile = [](std::string_view path)
    {
        std::ifstream file{ path.data() };
        std::string returnString { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        return returnString;
    };

    auto LoadShader = [&LoadFile](GLuint shader_id, std::string_view path, bool isFrag = false)
    {
        //Prevent dangling pointer by moving it to local variable
        std::string returnString = std::move(LoadFile(path));
        const GLchar* shader_contents;

        if (!isShaderText || !isFrag)
        {
            shader_contents = returnString.data();
            if (!isShaderText)
                strcpy(shaderText, returnString.c_str());
        }
        else
        {
            shader_contents = shaderText;
        }
            

        glShaderSource(shader_id, 1, &shader_contents, nullptr);
        glCompileShader(shader_id);

        GLint success;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::string infoLog;
            const GLsizei infoLength = 512;
            infoLog.resize(infoLength + 1, '\0');
            glGetShaderInfoLog(shader_id, infoLength, nullptr, infoLog.data());
        }
    };
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    LoadShader(vertex_shader, vertex_shader_path);
    LoadShader(fragment_shader, fragment_shader_path, true);
}

SandboxApplication::ShaderProgram::ShaderProgram()
{
    shader_id = glCreateProgram();
    //To Do: Check for errors
}

SandboxApplication::ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(shader_id);     
}

GLuint SandboxApplication::ShaderProgram::GetShader()
{
    //Read-only... but OpenGL is a global state machine so who cares honestly
    return shader_id; 
}

void SandboxApplication::Pipeline::AttachToShader(ShaderProgram& shader) const
{
    GLuint shader_id = shader.GetShader();
    glAttachShader(shader_id, vertex_shader);
    glAttachShader(shader_id, fragment_shader);

    //To Do: Error checking
    glLinkProgram(shader.GetShader());
}

SandboxApplication::Pipeline::~Pipeline()
{
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void SandboxApplication::ShaderProgram::BeginDraw()
{
    glUseProgram(shader_id);
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

    //https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#glbuffer
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, sizeof(positions), positions.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 3);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
}

SandboxApplication::ExampleTriangle::~ExampleTriangle()
{
    glDeleteVertexArrays(1, &vao);
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

    shader_program.emplace();
    triangle.emplace();
    basic_pipeline.emplace("./data/shaders/triangle.vert.glsl", "./data/shaders/triangle.frag.glsl");
    basic_pipeline->AttachToShader(*shader_program);

    return true;
}

void SandboxApplication::Update(double dt)
{
    if (IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        Close();
    }

    if (IsKeyPressed(GLFW_KEY_Q))
    {
        isShaderText = true;
        Load();
    }

}

void SandboxApplication::RenderScene(double dt)
{
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    triangle->Draw(*shader_program);
}

void SandboxApplication::RenderUI(double dt)
{
    ImGui::Begin("Window");
    {
        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImGui::InputTextMultiline("##source", shaderText, IM_ARRAYSIZE(shaderText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
        ImGui::End();
    }

    //ImGui::ShowDemoWindow();
}