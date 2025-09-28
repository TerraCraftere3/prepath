#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Context.h"

namespace Prepath
{

    class Shader
    {
    public:
        Shader();
        ~Shader();
        void bind();

        static std::shared_ptr<Shader> generateShader(const char *vertexSource, const char *fragmentSource);
        static std::shared_ptr<Shader> generateShader(const char *vertexSource, const char *geometrySource, const char *fragmentSource);

        void setUniform1i(const std::string &name, int value);
        void setUniform1f(const std::string &name, float value);
        void setUniform2f(const std::string &name, const glm::vec2 &value);
        void setUniform2f(const std::string &name, float x, float y);
        void setUniform3f(const std::string &name, const glm::vec3 &value);
        void setUniform3f(const std::string &name, float x, float y, float z);
        void setUniform4f(const std::string &name, const glm::vec4 &value);
        void setUniform4f(const std::string &name, float x, float y, float z, float w);
        void setUniformMat3f(const std::string &name, const glm::mat3 &matrix);
        void setUniformMat4f(const std::string &name, const glm::mat4 &matrix);
        void setUniformMat4fArray(const std::string &name, const std::vector<glm::mat4> &matrices);

    private:
        GLint getUniformLocation(const std::string &name) const;
        void setupShader(const char *vertexSource, const char *fragmentSource);
        void setupShader(const char *vertexSource, const char *geometrySource, const char *fragmentSource);

    private:
        mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
        GLuint m_ShaderProgram;
    };

}