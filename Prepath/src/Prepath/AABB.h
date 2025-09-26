#pragma once
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::mat4
#include <array>

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

        /// Transform this AABB with a matrix (returns a new AABB)
        AABB operator*(const glm::mat4 &transform) const
        {
            // All 8 corners of the AABB
            std::array<glm::vec3, 8> corners = {
                glm::vec3(min.x, min.y, min.z),
                glm::vec3(max.x, min.y, min.z),
                glm::vec3(min.x, max.y, min.z),
                glm::vec3(max.x, max.y, min.z),
                glm::vec3(min.x, min.y, max.z),
                glm::vec3(max.x, min.y, max.z),
                glm::vec3(min.x, max.y, max.z),
                glm::vec3(max.x, max.y, max.z)};

            glm::vec3 newMin(std::numeric_limits<float>::max());
            glm::vec3 newMax(-std::numeric_limits<float>::max());

            for (auto &corner : corners)
            {
                glm::vec4 transformed = transform * glm::vec4(corner, 1.0f);
                glm::vec3 pos = glm::vec3(transformed) / transformed.w; // handle perspective divide if needed

                newMin = glm::min(newMin, pos);
                newMax = glm::max(newMax, pos);
            }

            return AABB(newMin, newMax);
        }

        AABB &operator*=(const glm::mat4 &transform)
        {
            *this = *this * transform;
            return *this;
        }
    };
}
