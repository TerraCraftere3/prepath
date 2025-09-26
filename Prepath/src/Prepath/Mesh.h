#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Material.h"
#include "AABB.h"

namespace Prepath
{

    class Mesh
    {
    public:
        Mesh();
        ~Mesh();

        // Non-copyable
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        // Movable
        Mesh(Mesh &&other) noexcept;
        Mesh &operator=(Mesh &&other) noexcept;

        void draw() const;

        // ---- Creation Methods ----
        static std::shared_ptr<Mesh> generateMesh(
            const std::vector<glm::vec3> &positions,
            const std::vector<glm::vec3> &normals,
            const std::vector<glm::vec2> &texCoords,
            const std::vector<glm::vec3> *tangentsIn = nullptr,
            const std::vector<glm::vec3> *bitangentsIn = nullptr);
        static std::shared_ptr<Mesh> generateCube(float size = 1.0f);
        static std::shared_ptr<Mesh> generateQuad(float width = 1.0f, float height = 1.0f);

        // ---- Statistics Methods ----
        GLsizei getVertexCount() const { return vertexCount; }
        GLsizei getTriangleCount() const { return triangleCount; }
        GLsizei getDrawCallCount() const { return drawCallCount; }

    public:
        AABB bounds;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        std::shared_ptr<Material> material;

    private:
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLsizei vertexCount = 0;
        GLsizei triangleCount = 0;
        GLsizei drawCallCount = 0;

        void setupMesh(
            const std::vector<glm::vec3> &positions,
            const std::vector<glm::vec3> &normals,
            const std::vector<glm::vec2> &texCoords,
            const std::vector<glm::vec3> *tangentsIn = nullptr,
            const std::vector<glm::vec3> *bitangentsIn = nullptr);
    };
}