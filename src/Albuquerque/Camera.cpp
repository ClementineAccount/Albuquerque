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
        camTarget = glm::vec3(0.0f, 0.0f, 0.0f); //Target the origin
        
        //What is even the point of having it initalized twice like this tho? You already have it as a default as member 
        camUp = glm::vec3(0.0f, 1.0f, 0.0f);

        camForward = glm::normalize(camTarget - camPos);

        CalibrateDirectional();
    }

    void Camera::CalibrateDirectional()
    {
        //To Do: Move this const somewhere else
        static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        //it actually points behind the camera's head because we live in a society
        camForward = glm::normalize(camForward);
        camRight = glm::cross(camForward, worldUp);
        camUp = glm::cross(camRight, camForward);
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

    void Camera::MoveFly(directionalInput moveDirection, float speed)
    {
        //This moves both position and target
        if (moveDirection == directionalInput::moveUp)
        {
            camPos += camUp * speed;
            camTarget += camUp * speed;
        }
        else if (moveDirection == directionalInput::moveDown)
        {
            camPos -= camUp * speed;
            camTarget -= camUp * speed;
        }
        else if (moveDirection == directionalInput::moveLeft)
        {
            camPos -= camRight * speed;
            camTarget -= camRight * speed;
        }
        else if (moveDirection == directionalInput::moveRight)
        {
            camPos += camRight * speed;
            camTarget += camRight * speed;
        }
        else if (moveDirection == directionalInput::moveForward)
        {
            camPos += camForward * speed;
            camTarget += camForward * speed;
        }
        else if (moveDirection == directionalInput::moveBack)
        {
            camPos -= camForward * speed;
            camTarget -= camForward * speed;
        }
    }

    //There is no lerp for this at the moment
    void Camera::RotateFly(float yawDegreesAdd, float pitchDegreesAdd)
    {
        this->yawDegrees += yawDegreesAdd;
        this->pitchDegrees += pitchDegreesAdd;

        if (this->pitchDegrees > 89.0f) this->pitchDegrees = 89.0f;
        if (this->pitchDegrees < -89.0f) this->pitchDegrees = -89.0f;

        //Actually there is another route... instead of setting the target directly I can calculate the euler angle rotation
        //rotation relative to the forward of (1.0, 0.0, 0.0) I guess?
        //Its not as good as doing quaterions or whatever but I can try it just to see if it works as an experiment

        //Don't really know how to do this system if the forward is not implied to start at origin sadly. Would probably
        //have to use quaterions instead or get better at this type of math
        //I will just dump this here and use it to learn the proper math some other time: 
        //https://www.youtube.com/watch?v=MZuYmG1GBFk
        camForward.x = cos(glm::radians(this->yawDegrees)) * cos(glm::radians(this->pitchDegrees));
        camForward.y = sin(glm::radians(this->pitchDegrees));
        camForward.z = sin(glm::radians(this->yawDegrees)) * cos(glm::radians(this->pitchDegrees));

        CalibrateDirectional();
        camTarget = camPos + camForward;
    }

}

