#ifndef SMOL_SYSTEMS_ROOT
#define SMOL_SYSTEMS_ROOT

#include <smol/smol_keyboard.h>

namespace smol
{
  class Renderer;
  struct Scene;
  struct Config;

  struct SystemsRoot
  {
    Config* config;
    Renderer* renderer;
    Keyboard* keyboard; 
    Scene* loadedScene;    //TODO: Remove this. I just need some place to get a reference to the scene from the game side. This will probably be a scene manager in the future.
  };
}

#endif  // SMOL_SYSTEMS_ROOT
