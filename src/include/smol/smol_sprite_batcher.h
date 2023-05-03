#ifndef SMOL_SPRITE_BATCHER_H
#define SMOL_SPRITE_BATCHER_H

#include <smol/smol_engine.h>
#include <smol/smol_renderable.h>
#include <smol/smol_handle_list.h>
namespace smol
{
  struct Scene;
  struct SMOL_ENGINE_API SpriteBatcher final
  {
    enum Mode
    {
      SCREEN = 0,
      CAMERA = 1
    };

    Mode mode;
    Handle<Renderable> renderable;
    Arena arena;
    int spriteCount;
    int spriteCapacity;
    bool dirty;

    static const size_t positionsSize;
    static const size_t indicesSize;
    static const size_t colorsSize;
    static const size_t uvsSize;
    static const size_t totalSpriteSize;

    SpriteBatcher(Handle<Material> material, Mode mode, int capacity);
    SpriteBatcher() = delete;
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::SpriteBatcher>;
  template class SMOL_ENGINE_API smol::Handle<smol::SpriteBatcher>;
}

#endif  //SMOL_SPRITE_BATCHER_H

