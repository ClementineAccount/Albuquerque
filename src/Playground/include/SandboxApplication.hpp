#pragma once

#include <glad/glad.h>

#include <Albuquerque/Application.hpp>
#include <Albuquerque/Camera.hpp>
#include <Albuquerque/DrawObject.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
#include <array>
#include <optional>



//Sandbox Application that doesn't have Fwog stuff.

class SandboxApplication final : public Albuquerque::Application
{
public:

protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;
    bool Load() override;
    void RenderScene(double dt) override;
    void RenderUI(double dt) override;
    void Update(double dt) override;

    //void UpdateViewBuffers(Albuquerque::Camera const& camera);


protected:

    class ShaderProgram
    {
    public:
        ShaderProgram();
        void Draw();
        
        GLuint GetShader();

    private:
        GLuint shader_program;
    };

    class Pipeline
    {

    public:
        Pipeline() = delete;
        Pipeline(std::string_view vertex_shader_path, std::string_view fragment_shader_path);

        void AttachToShader(ShaderProgram& shader) const;

    private:
        GLuint vertex_shader = 0;
        GLuint fragment_shader = 0;
    };


private:


};