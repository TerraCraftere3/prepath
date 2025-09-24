#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <format>
#include <glad/glad.h>

#include "Context.h"
#include "Scene.h"
#include "Camera.h"

namespace Prepath
{

    struct RenderSettings
    {
        int width;
        int height;
        Camera cam;
        RenderSettings();
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void render(const Scene &scene, const RenderSettings &settings);

    private:
        GLuint m_ShaderProgram;
    };

}