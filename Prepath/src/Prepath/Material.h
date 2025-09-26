#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Context.h"
#include "Texture.h"

namespace Prepath
{

    struct Material
    {
        glm::vec3 tint = glm::vec3(1.0f);
        std::shared_ptr<Texture> albedo;

        static inline std::shared_ptr<Material> createMaterial()
        {
            auto mat = std::make_shared<Material>();
            unsigned char defaultAlbedo[3] = {255, 0, 0};
            mat->albedo = Texture::generateTexture(defaultAlbedo, 1, 1, 3);
            return mat;
        }
    };

}