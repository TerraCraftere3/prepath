#include "Light.h"
#include "Error.h"
#include <iostream>

namespace Prepath
{
    void PointLight::setupLight() {}

    std::shared_ptr<PointLight> Prepath::Light::generatePointLight()
    {
        auto light = std::make_shared<PointLight>();
        light->setupLight();
        return light;
    }
}