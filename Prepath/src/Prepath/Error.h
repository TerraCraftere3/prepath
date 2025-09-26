#pragma once
#include <glad/glad.h>
#include "Context.h"

namespace Prepath
{
    void GLAPIENTRY
    OpenGLErrorCallback(GLenum source,
                        GLenum type,
                        GLuint id,
                        GLenum severity,
                        GLsizei length,
                        const GLchar *message,
                        const void *userParam);
}