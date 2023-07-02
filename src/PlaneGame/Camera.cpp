#pragma once
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <Fwog/Buffer.h>

namespace PlaneGame
{
	Camera::Camera() 
        : cameraUniformsBuffer(Fwog::BufferStorageFlag::DYNAMIC_STORAGE),
          cameraUniformsSkyboxBuffer(Fwog::BufferStorageFlag::DYNAMIC_STORAGE)
	{
        //To Do: Have this pi constant declared somewhere else
        static constexpr float PI = 3.1415926f;

		//Create the buffers (could refactor this to initalization list in the future)

        //To Do: Allow these positions to be set? 
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

        cameraUniformsBuffer.SubData(cameraStruct, 0);

        //Update();
	}
}

