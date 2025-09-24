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
#include "Shader.h"

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
        std::shared_ptr<Shader> m_Shader;
    };

}