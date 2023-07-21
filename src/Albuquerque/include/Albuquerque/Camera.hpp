#pragma once 
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <Fwog/Buffer.h>
#include <optional>

namespace Albuquerque
{

    class Camera
    {
    public:
        Camera();

        glm::vec3 camPos = glm::vec3(3.0f, 3.0f, 3.0f);
        glm::vec3 camTarget = glm::vec3(0.0f, 0.0f, 0.0f);

        glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 camRight = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 camForward = glm::vec3(0.0f, 0.0f, 1.0f);

        float yawDegrees = -90.0f;
        float pitchDegrees = 0.0f;

        float nearPlane = 0.01f;
        float farPlane = 5000.0f;


        //Accounts for y-axis pointing down coordinate systems (such as for 2D cameras)
        //To Do: Make this private and make user have to toggle vertical state via function
        uint32_t flipVertical = 1;

        //Camera controls. Could probably move to some other controller in the future but whatever
        enum class directionalInput
        {
            moveUp,
            moveDown,
            moveLeft,
            moveRight,
            moveForward,
            moveBack
        };

        //Maintains the current target that it previously had. The caller is expected to correct the speed for deltaTime
        //prior to passsing it in.
        void MoveArcball(directionalInput moveDirection, float speed = 1.0f);

        void MoveFly(directionalInput moveDirection, float speed = 1.0f);

        //Allow rotation of camera using mouse or keyboard. Lets try prototyping keyboard version as I haven't done that before
        void RotateFly(float yawDegreesAdd, float pitchDegreesAdd);

    private:
        void CalibrateDirectional();

    };
}
