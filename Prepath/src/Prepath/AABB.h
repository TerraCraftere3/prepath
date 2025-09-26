#pragma once
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp> // Using glm for vec3

namespace Prepath
{
    class AABB
    {
    public:
        glm::vec3 min;
        glm::vec3 max;

        AABB()
            : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)) {}

        AABB(const glm::vec3 &a, const glm::vec3 &b)
            : min(glm::min(a, b)), max(glm::max(a, b)) {}

        AABB(const AABB &a, const AABB &b)
        {
            min = glm::min(a.min, b.min);
            max = glm::max(a.max, b.max);
        }
    };
}