#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

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

        // Static method to create a mesh
        static std::shared_ptr<Mesh> generateMesh(
            const std::vector<glm::vec3> &positions,
            const std::vector<glm::vec3> &normals,
            const std::vector<glm::vec2> &texCoords);
        static std::shared_ptr<Mesh> generateCube(float size = 1.0f);
        static std::shared_ptr<Mesh> generateQuad(float width = 1.0f, float height = 1.0f);

    public:
        glm::mat4 modelMatrix = glm::mat4(1.0f);

    private:
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLsizei vertexCount = 0;

        void setupMesh(
            const std::vector<glm::vec3> &positions,
            const std::vector<glm::vec3> &normals,
            const std::vector<glm::vec2> &texCoords);
    };
}