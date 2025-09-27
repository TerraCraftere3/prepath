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
        std::vector<std::shared_ptr<Mesh>> &getMeshes() { return m_Meshes; }
        const std::vector<std::shared_ptr<Mesh>> &getMeshes() const { return m_Meshes; }

    public:
        std::shared_ptr<Cubemap> skybox;
        glm::vec3 lightDir;
        AABB bounds;

    private:
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
    };

}