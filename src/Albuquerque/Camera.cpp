#include "Albuquerque/Camera.hpp"
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

#include <Fwog/Buffer.h>
#include <iostream>

namespace Albuquerque
{
    Camera::Camera()
    {
        nearPlane = 0.01f;
        farPlane = 5000.0f;

        camPos = glm::vec3(3.0f, 3.0f, 3.0f);
        target = glm::vec3(0.0f, 0.0f, 0.0f); //Target the origin
        up = glm::vec3(0.0f, 1.0f, 0.0f);

    }

    void Camera::MoveArcball(directionalInput moveDirection, float speed)
    {
        //To Do: Account for alternate coordinate systems

        if (moveDirection == directionalInput::moveUp)
        {
            camPos.y += speed * flipVertical;
        }
        else if (moveDirection == directionalInput::moveDown)
        {
            camPos.y -= speed * flipVertical;
        }
        else if (moveDirection == directionalInput::moveLeft)
        {
            camPos.z += speed;
        }
        else if (moveDirection == directionalInput::moveRight)
        {
            camPos.z -= speed;
        }
        else if (moveDirection == directionalInput::moveForward)
        {
            camPos.x -= speed;
        }
        else if (moveDirection == directionalInput::moveBack)
        {
            camPos.x += speed;
        }

        //Switch cases have really ugly syntaxes and compiler optimizations turn both if/else and switch cases to jump tables
        //so forget it
        //switch (moveDirection)
        //{
        //directionalInput::moveUp:
        //camPos.x += speed;
        //    break;

        //default:
        //    break;
        //}

    }

}

