#ifndef SMOL_SPRITE_BATCHER_H
#define SMOL_SPRITE_BATCHER_H

#include <smol/smol_engine.h>
#include <smol/smol_renderable.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_vector2.h>

namespace smol
{
  struct Scene;
  struct Color;
  struct SceneNode;

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
    int nodeCount;        // how many nodes are handled by this sprite batcher
    int spriteCount;      // how many sprites are handled by this batcher. Nodes can push multiple sprites
    int batchedSprites;   // how many sprites are batched so far. Batches happen between begin() and end()
    int spriteCapacity;
  
    //TODO(marcio): make these members private
    Vector3*  positions;
    bool      dirty;
    char*     memory;
    Color*    colors;
    Vector2*  uvs;
    uint32*   indices;

    // we cache the texture dimentions to adjust sprite UVS
    Vector2 textureDimention;


    static const size_t positionsSize;
    static const size_t indicesSize;
    static const size_t colorsSize;
    static const size_t uvsSize;
    static const size_t totalSpriteSize;

    SpriteBatcher(Handle<Material> material, Mode mode, int capacity);
    SpriteBatcher() = delete;
    size_t getTotalSizeRequired() const;

    void begin();
    void pushSpriteNode(SceneNode* sceneNode);
    void end();
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::SpriteBatcher>;
  template class SMOL_ENGINE_API smol::Handle<smol::SpriteBatcher>;
}

#endif  //SMOL_SPRITE_BATCHER_H

