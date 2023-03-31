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
    static const size_t positionsSize;
    static const size_t indicesSize;
    static const size_t colorsSize;
    static const size_t uvsSize;
    static const size_t totalSpriteSize;

    Handle<Renderable> renderable;
    Arena arena;
    int spriteCount;
    int spriteCapacity;
    bool dirty;

    SpriteBatcher(Handle<Material> material, int capacity);
    SpriteBatcher() = delete;
  };
}

#endif  //SMOL_SPRITE_BATCHER_H

