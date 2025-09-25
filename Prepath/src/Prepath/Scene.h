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
#include "Context.h"

namespace Prepath
{

    class Scene
    {
    public:
        Scene() = default;

        // ---- Meshes ----
        void addMesh(std::shared_ptr<Mesh> mesh) { m_Meshes.push_back(mesh); }
        std::vector<std::shared_ptr<Mesh>> &getMeshes() { return m_Meshes; }
        const std::vector<std::shared_ptr<Mesh>> &getMeshes() const { return m_Meshes; }

    public:
        glm::vec3 lightDir;

    private:
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
    };

}