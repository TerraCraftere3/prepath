#include "Renderer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

namespace Prepath
{
    Renderer::Renderer()
    {
        m_Shader = Shader::generateShader(PREPATH_READ_SHADER("default.vert").c_str(), PREPATH_READ_SHADER("default.frag").c_str());
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::render(const Scene &scene, const RenderSettings &settings)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, settings.width, settings.height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->bind();
        glm::mat4 view = settings.cam.getViewMatrix();
        m_Shader->setUniformMat4f("uView", view);
        glm::mat4 projection = settings.cam.getProjectionMatrix(float(settings.width) / settings.height);
        m_Shader->setUniformMat4f("uProjection", projection);

        m_Statistics.drawCallCount = 0;
        m_Statistics.triangleCount = 0;
        m_Statistics.vertexCount = 0;

        for (auto mesh : scene.getMeshes())
        {
            m_Shader->setUniformMat4f("uModel", mesh->modelMatrix);
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(mesh->modelMatrix)));
            m_Shader->setUniformMat3f("uNormalMatrix", normalMatrix);
            if (mesh->material)
            {
                auto mat = mesh->material;
                m_Shader->setUniform3f("uTint", mat->tint);
            }
            mesh->draw();
            m_Statistics.drawCallCount += mesh->getDrawCallCount();
            m_Statistics.triangleCount += mesh->getTriangleCount();
            m_Statistics.vertexCount += mesh->getVertexCount();
        }

        GLenum error;
        do
        {
            error = glGetError();
            if (error != GL_NO_ERROR)
            {
                PREPATH_LOG_ERROR("OpenGL error: {}", error);
            }
        } while (error != GL_NO_ERROR);
    }

    RenderSettings::RenderSettings()
    {
        this->width = 800; // Default Values
        this->height = 600;
        this->cam = Camera();
    }
} // namespace Prepath