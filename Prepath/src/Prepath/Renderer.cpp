#include "Renderer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

namespace Prepath
{
    const char *vertexShaderSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec2 aTexCoord;

    uniform mat4 uMVP;

    out vec3 Normal;
    out vec3 Position;
    out vec2 TexCoord;

    void main()
    {
        gl_Position = uMVP * vec4(aPos, 1.0);
        Normal = aNormal;
        Position = aPos;
        TexCoord = aTexCoord;
    }
)";

    const char *fragmentShaderSrc = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 Normal;    
    in vec3 Position;
    in vec2 TexCoord;

    void main()
    {
        FragColor = vec4(TexCoord, 0.0, 1.0);
    }
)";

    Renderer::Renderer()
    {
        m_Shader = Shader::generateShader(vertexShaderSrc, fragmentShaderSrc);
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::render(const Scene &scene, const RenderSettings &settings)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glDisable(GL_CULL_FACE);
        /*glCullFace(GL_BACK);
        glFrontFace(GL_CCW);*/

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, settings.width, settings.height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->bind();
        glm::mat4 view = settings.cam.getViewMatrix();
        glm::mat4 projection = settings.cam.getProjectionMatrix(float(settings.width) / settings.height);

        m_Statistics.drawCallCount = 0;
        m_Statistics.triangleCount = 0;
        m_Statistics.vertexCount = 0;

        for (auto mesh : scene.getMeshes())
        {
            glm::mat4 mvp = projection * view * mesh->modelMatrix;
            m_Shader->setUniformMat4f("uMVP", mvp);
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