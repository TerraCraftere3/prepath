#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <format>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Cubemap.h"
#include "Light.h"
#include "Context.h"

namespace Prepath
{

    class Scene
    {
    public:
        Scene();

        // ---- Meshes ----
        void updateBounds();
        void addMesh(std::shared_ptr<Mesh> mesh)
        {
            m_Meshes.push_back(mesh);
            updateBounds();
        }
        void addPointLight(std::shared_ptr<PointLight> light)
        {
            m_PointLights.push_back(light);
        }
        std::vector<std::shared_ptr<Mesh>> &getMeshes() { return m_Meshes; }
        const std::vector<std::shared_ptr<Mesh>> &getMeshes() const { return m_Meshes; }
        std::vector<std::shared_ptr<PointLight>> &getPointLights() { return m_PointLights; }
        const std::vector<std::shared_ptr<PointLight>> &getPointLights() const { return m_PointLights; }

    public:
        std::shared_ptr<Cubemap> skybox;
        bool hasSkyLight = true;
        glm::vec3 lightDir;
        AABB bounds;

    private:
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
        std::vector<std::shared_ptr<PointLight>> m_PointLights;
    };

}