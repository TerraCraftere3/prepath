#include "Scene.h"

namespace Prepath
{
    Scene::Scene()
    {
        static unsigned char skyBlue[3] = {135, 206, 235};

        unsigned char *faces[6] = {
            skyBlue, skyBlue, skyBlue,
            skyBlue, skyBlue, skyBlue};

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