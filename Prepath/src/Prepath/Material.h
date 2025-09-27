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
        std::shared_ptr<Texture> normal;
        std::shared_ptr<Texture> roughness;
        std::shared_ptr<Texture> metal;
        std::shared_ptr<Texture> ao;

        static inline std::shared_ptr<Material> generateMaterial()
        {
            auto mat = std::make_shared<Material>();
            unsigned char defaultAlbedo[3] = {255, 255, 255}; // white
            mat->albedo = Texture::generateTexture(defaultAlbedo, 1, 1, 3);

            unsigned char defaultNormal[3] = {127, 127, 255}; // flat normal
            mat->normal = Texture::generateTexture(defaultNormal, 1, 1, 3);

            unsigned char defaultRoughness[1] = {255}; // fully rough
            mat->roughness = Texture::generateTexture(defaultRoughness, 1, 1, 1);

            unsigned char defaultMetal[1] = {0}; // non-metal
            mat->metal = Texture::generateTexture(defaultMetal, 1, 1, 1);

            unsigned char defaultAO[1] = {255}; // no occlusion
            mat->ao = Texture::generateTexture(defaultAO, 1, 1, 1);
            return mat;
        }
    };

}