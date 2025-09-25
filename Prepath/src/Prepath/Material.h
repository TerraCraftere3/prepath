#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Context.h"

namespace Prepath
{

    struct Material
    {
        glm::vec3 tint = glm::vec3(1.0f);

        static inline std::shared_ptr<Material> createMaterial()
        {
            auto mat = std::make_shared<Material>();
            return mat;
        }
    };

}