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
    void render(float deltaTime);
    Scene& getCurrentScene();
  };

}
#endif //SMOL_SCENE_NAMAGER_H
