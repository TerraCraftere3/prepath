#include "Scene.h"

namespace Prepath
{
    void Scene::updateBounds()
    {
        bounds = AABB();
        for (auto mesh : m_Meshes)
        {
            bounds = AABB(bounds, mesh->bounds);
        }
    }
}