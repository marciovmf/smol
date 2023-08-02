
#include "include/smol/smol_scene_manager.h"
#include <smol/smol_scene_manager.h>
#include <smol/smol_scene.h>

//TODO(marcio): The ide of this class is to facilitate loading and unloading scenes from and to file. For now, it just provides us a scene and a way to render it.

namespace smol
{
  SceneManager::SceneManager()
  {
    scene = new Scene();
  }

  SceneManager::~SceneManager()
  {
    delete scene;
  }

  Scene& SceneManager::getCurrentScene()
  {
    return *scene;
  };

  void SceneManager::renderScene(float deltaTime)
  {
    scene->render(deltaTime);
  }

  SceneManager& SceneManager::get()
  {
    static SceneManager instance;
    return instance;
  }

  void SceneManager::cleanupScene()
  {
    scene->~Scene();
    scene = new Scene();
  }
}
