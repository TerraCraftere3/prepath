#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Material.h"
#include "AABB.h"

namespace Prepath
{
    class PointLight
    {
    public:
        PointLight() = default;
        virtual ~PointLight() = default;

        PointLight(const PointLight &) = delete;
        PointLight &operator=(const PointLight &) = delete;

        PointLight(PointLight &&) noexcept = default;
        PointLight &operator=(PointLight &&) noexcept = default;

        bool hidden = false;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;

    protected:
        void setupLight();

        friend class Light;
    };

    class Light
    {
    public:
        static std::shared_ptr<PointLight> generatePointLight();
    };

} // namespace Prepath
