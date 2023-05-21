#include <smol/smol_renderer_types.h>
#include <smol/smol_text_node.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_scene.h>

namespace smol
{
  Handle<SceneNode> TextNode::create(
      Handle<SpriteBatcher> batcher,
      Handle<Font> font,
      const Vector3& position,
      const char* text,
      const Color& color,
      Handle<SceneNode> parent)
  {
    Transform t(
        position,
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        parent);

    Scene& scene = SystemsRoot::get()->sceneManager.getLoadedScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::TEXT, t);
    handle->text.batcher = batcher;
    handle->text.setText(text);
    batcher->nodeCount++;
    batcher->dirty = true;
    handle->text.node = handle;
    return handle;
  }

  void TextNode::setText(const char* text)
  {
    size_t textLen = strlen(text);
    size_t memSize = textLen + 1 + textLen * sizeof(VertexPCU);
    if (arena.getCapacity() == 0)
    {
      arena.initialize(memSize);
    }
    //arena.reset();
    //char* memory =  (char*) arena.pushSize(textLen);
    //...
  }

  const char* TextNode::getText() const
  {
    return text;
  }

  void TextNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::TEXT), "Handle passed to TextNode::destroy() is not of type TEXT");
    SystemsRoot::get()->sceneManager.getLoadedScene().destroyNode(handle);
  }
}
