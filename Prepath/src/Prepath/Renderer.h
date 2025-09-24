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

namespace Prepath
{

    class Renderer
    {
    public:
        Renderer();
        void render(const Scene &scene);
    };

}