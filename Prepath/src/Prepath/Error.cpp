#include "Error.h"

const char *GetSourceString(GLenum source)
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "Application";
    case GL_DEBUG_SOURCE_OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

const char *GetTypeString(GLenum type)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "Deprecated Behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "Undefined Behavior";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "Performance";
    case GL_DEBUG_TYPE_MARKER:
        return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "Push Group";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "Pop Group";
    case GL_DEBUG_TYPE_OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

const char *GetSeverityString(GLenum severity)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "Medium";
    case GL_DEBUG_SEVERITY_LOW:
        return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "Notification";
    default:
        return "Unknown";
    }
}

void GLAPIENTRY Prepath::OpenGLErrorCallback(GLenum source, GLenum type, GLuint id,
                                             GLenum severity, GLsizei length,
                                             const GLchar *message, const void *userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        PREPATH_LOG_ERROR(
            "---- OPENGL ERROR ----\n\tSource: {}\n\tType: {}\n\tSeverity: {}\n\tID: {}\n\tMessage: {}",
            GetSourceString(source),
            GetTypeString(type),
            GetSeverityString(severity),
            id,
            message);
    }
}
