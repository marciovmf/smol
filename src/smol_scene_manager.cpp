
#include <smol/smol_scene_manager.h>
#include <smol/smol_scene.h>

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

  Scene& SceneManager::getLoadedScene()
  {
    return *scene;
  };

}
