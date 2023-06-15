#ifndef SMOL_SCENE_H
#define SMOL_SCENE_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_transform.h>
#include <smol/smol_color.h>
#include <smol/smol_scene_node.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_sprite_batcher.h>

namespace smol
{
  struct Image;
  struct MeshData;
  struct ResourceManager;
  struct SMOL_ENGINE_API Scene final
  {
    private:

      HandleList<Renderable> renderables;
      HandleList<SceneNode> nodes;
      HandleList<SpriteBatcher> batchers;
      Arena renderKeys;
      Arena renderKeysSorted;
      Handle<smol::Texture> defaultTexture;
      Handle<smol::ShaderProgram> defaultShader;
      Handle<smol::Material> defaultMaterial;
      Mat4 viewMatrix;
      const smol::SceneNode nullSceneNode;

    public:
      Scene();
      ~Scene();

      //
      // Create / Destroy scene resources
      //

      Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
      void destroyRenderable(Handle<Renderable> handle);
      void destroyRenderable(Renderable* renderable);

      Handle<SpriteBatcher> createSpriteBatcher(Handle<Material> material, int capacity = 32);
      void setSpriteBatcherMode(Handle<SpriteBatcher> handle);
      void destroySpriteBatcher(Handle<SpriteBatcher> handle);

      int getNodeCount() const;
      const SceneNode* getNodes(uint32* count = nullptr) const;

#ifndef SMOL_MODULE_GAME
      Handle<SceneNode> createNode(SceneNode::Type type, const Transform& transform);
      void destroyNode(Handle<SceneNode> handle);
#endif

      //
      // Render
      //
      void render(float deltaTime);
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H
