#include "Shader.h"

namespace Prepath
{
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

    GLuint createShaderProgram(const char *vertexSource, const char *fragmentSource)
    {
        GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

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

    Shader::Shader() {}
    Shader::~Shader()
    {
        if (m_ShaderProgram)
            glDeleteProgram(m_ShaderProgram);
    }

    void Shader::bind()
    {
        glUseProgram(m_ShaderProgram);
    }
    std::shared_ptr<Shader> Shader::generateShader(const char *vertexSource,
                                                   const char *fragmentSource)
    {
        auto shader = std::make_shared<Shader>();
        shader->setupShader(vertexSource, fragmentSource);
        return shader;
    }
    void Shader::setupShader(const char *vertexSource,
                             const char *fragmentSource)
    {
        m_ShaderProgram = createShaderProgram(vertexSource, fragmentSource);
    }

    GLint Shader::getUniformLocation(const std::string &name) const
    {
        auto it = m_UniformLocationCache.find(name);
        if (it != m_UniformLocationCache.end())
            return it->second;

        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
        if (location == -1)
        {
            PREPATH_LOG_ERROR("Warning: uniform '%s' doesn't exist!", name.c_str());
        }

        m_UniformLocationCache[name] = location;
        return location;
    }

    void Shader::setUniform1i(const std::string &name, int value)
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void Shader::setUniform1f(const std::string &name, float value)
    {
        glUniform1f(getUniformLocation(name), value);
    }

    void Shader::setUniform2f(const std::string &name, const glm::vec2 &value)
    {
        glUniform2f(getUniformLocation(name), value.x, value.y);
    }

    void Shader::setUniform2f(const std::string &name, float x, float y)
    {
        glUniform2f(getUniformLocation(name), x, y);
    }

    void Shader::setUniform3f(const std::string &name, const glm::vec3 &value)
    {
        glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
    }

    void Shader::setUniform3f(const std::string &name, float x, float y, float z)
    {
        glUniform3f(getUniformLocation(name), x, y, z);
    }

    void Shader::setUniform4f(const std::string &name, const glm::vec4 &value)
    {
        glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void Shader::setUniform4f(const std::string &name, float x, float y, float z, float w)
    {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }

    void Shader::setUniformMat3f(const std::string &name, const glm::mat3 &matrix)
    {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::setUniformMat4f(const std::string &name, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }
} // namespace Prepath