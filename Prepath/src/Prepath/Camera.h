#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Context.h"
#include "Scene.h"

namespace Prepath
{

    struct Camera
    {
        // Camera attributes
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Right = glm::normalize(glm::cross(Front, Up));
        glm::vec3 WorldUp = Up;

        // Euler angles
        float Yaw = -90.0f;
        float Pitch = 0.0f;

        // Camera options
        float MovementSpeed = 2.5f;
        float MouseSensitivity = 0.1f;
        float Zoom = 45.0f;

        // Constructor
        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f)
            : Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch)
        {
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() const
        {
            return glm::lookAt(Position, Position + Front, Up);
        }

        glm::mat4 getProjectionMatrix(float aspect, float nearPlane = 0.1f, float farPlane = 100.0f) const
        {
            return glm::perspective(glm::radians(Zoom), aspect, nearPlane, farPlane);
        }

        // Call AFTER changing yaw/pitch
        void updateCameraVectors()
        {
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Front = glm::normalize(front);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));
        }
    };

}