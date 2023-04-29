#ifndef SMOL_SCENE_NAMAGER_H
#define SMOL_SCENE_NAMAGER_H

#include <smol/smol_engine.h>

namespace smol
{
  struct Scene;

  struct SMOL_ENGINE_API SceneManager
  {
    private:
    Scene* scene;

    public:
    SceneManager();
    ~SceneManager();
    Scene& getLoadedScene();
  };

}
#endif SMOL_SCENE_NAMAGER_H
