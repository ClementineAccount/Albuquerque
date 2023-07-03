#pragma once 
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <Fwog/Buffer.h>
#include <optional>

class Camera
{
public:
    static constexpr float PI = 3.1415926f;

    Camera();
    void Update();

    glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float nearPlane = 0.01f;
    float farPlane = 5000.0f;

    struct CameraUniforms {
        glm::mat4 viewProj;
        glm::vec3 eyePos;
    };

    CameraUniforms cameraStruct;

    std::optional<Fwog::TypedBuffer<CameraUniforms>> cameraUniformsBuffer;

    //Skybox doesn't pass in the translation
    std::optional<Fwog::TypedBuffer<CameraUniforms>> cameraUniformsSkyboxBuffer;
};