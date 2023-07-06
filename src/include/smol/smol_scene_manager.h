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
    SceneManager();

    public:
    ~SceneManager();
    static SceneManager& get();
    void renderScene(float deltaTime);
    Scene& getCurrentScene();

    // Disallow copies
    SceneManager(const SceneManager& other) = delete;
    SceneManager(const SceneManager&& other) = delete;
    void operator=(const SceneManager& other) = delete;
    void operator=(const SceneManager&& other) = delete;
  };

}
#endif //SMOL_SCENE_NAMAGER_H
