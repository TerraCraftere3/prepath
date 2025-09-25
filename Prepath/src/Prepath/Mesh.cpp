#include "Mesh.h"
#include <iostream>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

namespace Prepath
{

    Mesh::Mesh() = default;

    Mesh::~Mesh()
    {
        if (VBO)
            glDeleteBuffers(1, &VBO);
        if (VAO)
            glDeleteVertexArrays(1, &VAO);
    }

    Mesh::Mesh(Mesh &&other) noexcept
    {
        VAO = other.VAO;
        VBO = other.VBO;
        vertexCount = other.vertexCount;

        other.VAO = 0;
        other.VBO = 0;
        other.vertexCount = 0;
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept
    {
        if (this != &other)
        {
            if (VBO)
                glDeleteBuffers(1, &VBO);
            if (VAO)
                glDeleteVertexArrays(1, &VAO);

            VAO = other.VAO;
            VBO = other.VBO;
            vertexCount = other.vertexCount;

            other.VAO = 0;
            other.VBO = 0;
            other.vertexCount = 0;
        }
        return *this;
    }

    void Mesh::draw() const
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    std::shared_ptr<Mesh> Mesh::generateMesh(
        const std::vector<glm::vec3> &positions,
        const std::vector<glm::vec3> &normals,
        const std::vector<glm::vec2> &texCoords)
    {
        auto mesh = std::make_shared<Mesh>();
        mesh->setupMesh(positions, normals, texCoords);
        return mesh;
    }

    void Mesh::setupMesh(
        const std::vector<glm::vec3> &positions,
        const std::vector<glm::vec3> &normals,
        const std::vector<glm::vec2> &texCoords)
    {
        if (positions.size() != normals.size() || positions.size() != texCoords.size())
        {
            std::cerr << "Mesh setup error: positions, normals, and texCoords must have same size!" << std::endl;
            return;
        }

        vertexCount = static_cast<GLsizei>(positions.size());
        triangleCount = vertexCount / 3.0;
        drawCallCount = 1;
        std::vector<Vertex> vertices(vertexCount);

        glm::vec3 minBound(FLT_MAX);
        glm::vec3 maxBound(-FLT_MAX);

        for (size_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].position = positions[i];
            vertices[i].normal = normals[i];
            vertices[i].texCoord = texCoords[i];

            minBound = glm::min(minBound, positions[i]);
            maxBound = glm::max(maxBound, positions[i]);
        }

        this->bounds.min = minBound;
        this->bounds.max = maxBound;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

        // TexCoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

        glBindVertexArray(0);
    }

    std::shared_ptr<Mesh> Mesh::generateCube(float size)
    {
        std::vector<glm::vec3> positions = {
            // Front face
            {-size, -size, size},
            {size, -size, size},
            {size, size, size},
            {-size, -size, size},
            {size, size, size},
            {-size, size, size},

            // Back face
            {-size, -size, -size},
            {-size, size, -size},
            {size, size, -size},
            {-size, -size, -size},
            {size, size, -size},
            {size, -size, -size},

            // Left face
            {-size, -size, -size},
            {-size, -size, size},
            {-size, size, size},
            {-size, -size, -size},
            {-size, size, size},
            {-size, size, -size},

            // Right face
            {size, -size, -size},
            {size, size, -size},
            {size, size, size},
            {size, -size, -size},
            {size, size, size},
            {size, -size, size},

            // Top face
            {-size, size, -size},
            {-size, size, size},
            {size, size, size},
            {-size, size, -size},
            {size, size, size},
            {size, size, -size},

            // Bottom face
            {-size, -size, -size},
            {size, -size, -size},
            {size, -size, size},
            {-size, -size, -size},
            {size, -size, size},
            {-size, -size, size},
        };

        std::vector<glm::vec3> normals = {
            // Front
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            // Back
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            // Left
            {-1, 0, 0},
            {-1, 0, 0},
            {-1, 0, 0},
            {-1, 0, 0},
            {-1, 0, 0},
            {-1, 0, 0},
            // Right
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            // Top
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0},
            // Bottom
            {0, -1, 0},
            {0, -1, 0},
            {0, -1, 0},
            {0, -1, 0},
            {0, -1, 0},
            {0, -1, 0}};

        std::vector<glm::vec2> texCoords = {
            {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}};

        return generateMesh(positions, normals, texCoords);
    }

    std::shared_ptr<Mesh> Mesh::generateQuad(float width, float height)
    {
        std::vector<glm::vec3> positions = {
            {-width / 2, 0.0f, -height / 2},
            {-width / 2, 0.0f, height / 2},
            {width / 2, 0.0f, -height / 2},

            {width / 2, 0.0f, -height / 2},
            {-width / 2, 0.0f, height / 2},
            {width / 2, 0.0f, height / 2}};

        std::vector<glm::vec3> normals(6, {0.0f, 1.0f, 0.0f});

        std::vector<glm::vec2> texCoords = {
            {0, 0},
            {0, 1},
            {1, 0},
            {1, 0},
            {0, 1},
            {1, 1}};

        return generateMesh(positions, normals, texCoords);
    }

}