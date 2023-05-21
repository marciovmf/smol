#ifndef SMOL_SPRITE_BATCHER_H
#define SMOL_SPRITE_BATCHER_H

#include <smol/smol_engine.h>
#include <smol/smol_renderable.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_vector2.h>
#include <smol/smol_stream_buffer.h>

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
    Handle<Material> material;
    int nodeCount;            // how many nodes are handled by this sprite batcher
    StreamBuffer buffer;
    bool      dirty;

    // we cache the texture dimentions to adjust sprite UVS
    Vector2 textureDimention;


    SpriteBatcher(Handle<Material> material, Mode mode, int capacity);
    SpriteBatcher() = delete;

    void begin();
    void pushSpriteNode(SceneNode* sceneNode);
    void end();
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::SpriteBatcher>;
  template class SMOL_ENGINE_API smol::Handle<smol::SpriteBatcher>;
}

#endif  //SMOL_SPRITE_BATCHER_H

