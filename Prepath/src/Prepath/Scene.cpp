#include "Scene.h"

namespace Prepath
{
    Scene::Scene()
    {
        static unsigned char skyDefault[3] = {0, 0, 0};

        unsigned char *faces[6] = {
            skyDefault, skyDefault, skyDefault,
            skyDefault, skyDefault, skyDefault};

        skybox = Cubemap::generateTexture(faces, 1, 1, 3);
    }

    void Scene::updateBounds()
    {
        bounds = AABB();
        for (auto mesh : m_Meshes)
        {
            if (!mesh->hidden)
                bounds = AABB(bounds, mesh->bounds * mesh->modelMatrix);
        }
    }
}