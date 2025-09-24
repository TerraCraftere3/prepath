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

    GLuint compileShader(GLenum type, const char *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            PREPATH_LOG_ERROR("Shader compilation error: {}", infoLog);
        }

        return shader;
    }

    GLuint createShaderProgram()
    {
        GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
        GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            PREPATH_LOG_ERROR("Shader linking error: {}", infoLog);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return program;
    }

    Renderer::Renderer()
    {
        m_ShaderProgram = createShaderProgram();
    }

    Renderer::~Renderer()
    {
        glDeleteProgram(m_ShaderProgram);
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

        glUseProgram(m_ShaderProgram);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                float(settings.width) / settings.height,
                                                0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
        glm::mat4 model = glm::mat4(1.0f);

        float angle = 1.0f;
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
        glm::mat4 mvp = projection * view * model;
        GLint loc = glGetUniformLocation(m_ShaderProgram, "uMVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

        for (auto mesh : scene.getMeshes())
        {
            mesh->draw();
        }

        glUseProgram(0);
    }

    RenderSettings::RenderSettings()
    {
        this->width = 800; // Default Values
        this->height = 600;
    }
} // namespace Prepath