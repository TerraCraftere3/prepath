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
        int width = 800;
        int height = 600;
        bool wireframe = false;
        bool culling = true;
        Camera cam;
        RenderSettings();
    };

    struct RenderStatistics
    {
        int drawCallCount = 0;
        int vertexCount = 0;
        int triangleCount = 0;
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void render(const Scene &scene, const RenderSettings &settings);
        RenderStatistics getStatistics() { return m_Statistics; }

    private:
        RenderStatistics m_Statistics;
        std::shared_ptr<Shader> m_Shader;
    };

}