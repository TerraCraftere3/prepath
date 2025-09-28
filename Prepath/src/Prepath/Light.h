#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Material.h"
#include "AABB.h"

#define PREPATH_SHADOWMAP_SIZE (1024)
#define PREPATH_CUBEMAP_POSX GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define PREPATH_CUBEMAP_NEGX GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1
#define PREPATH_CUBEMAP_POSY GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2
#define PREPATH_CUBEMAP_NEGY GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3
#define PREPATH_CUBEMAP_POSZ GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4
#define PREPATH_CUBEMAP_NEGZ GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5

namespace Prepath
{
    class PointLight
    {
    public:
        PointLight() = default;
        ~PointLight();

        PointLight(const PointLight &) = delete;
        PointLight &operator=(const PointLight &) = delete;

        PointLight(PointLight &&) noexcept = default;
        PointLight &operator=(PointLight &&) noexcept = default;

        GLenum copyCubemapFaceToTexture(GLenum face = PREPATH_CUBEMAP_POSX);

        bool hidden = false;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;

    protected:
        void setupLight();

        GLuint m_DepthTextureFace;
        GLuint m_DepthCubemap;
        GLuint m_DepthFramebuffer;

        friend class Light;
        friend class Renderer; // allow renderer to use protected stuff
    };

    class Light
    {
    public:
        static std::shared_ptr<PointLight> generatePointLight();
    };

} // namespace Prepath
