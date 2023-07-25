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