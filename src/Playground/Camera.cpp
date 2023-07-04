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

}
