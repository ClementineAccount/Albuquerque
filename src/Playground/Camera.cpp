#include "Camera.hpp"
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

#include <Fwog/Buffer.h>
#include <iostream>


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
