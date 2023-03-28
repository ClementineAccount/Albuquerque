#pragma once

#include <Project.Library/Application.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
//
//struct Vertex
//{
//    glm::vec3 Position;
//    glm::vec3 Normal;
//    glm::vec2 Uv;
//    glm::vec4 Tangent;
//};

//For draw_elements
struct draw_call
{
    bool drawArrays;
    uint32_t vao;
    uint32_t numIndices;
};



class ProjectApplication final : public Application
{
protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene() override;
    void RenderUI() override;
    void Update() override;

private:
    uint32_t _shaderProgram;
    draw_call helloPrim;
    
    bool MakeShader(std::string_view vertexShaderFilePath, std::string_view fragmentShaderFilePath);

};