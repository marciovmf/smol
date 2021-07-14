#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <smol/smol_engine.h>
#include <vector>

namespace smol
{
  struct SMOL_ENGINE_API Renderer
  {
    static void init(int width, int height);
    static void render();
  };
}

#endif  // SMOL_RENDERER_H
