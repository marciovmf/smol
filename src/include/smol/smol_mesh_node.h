#ifndef SMOL_MESH_NODE_H
#define SMOL_MESH_NODE_H

#include <smol/smol_scene_node_common.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>

namespace smol
{
  struct Renderable;

  struct SMOL_ENGINE_API MeshNode final : public NodeComponent
  {
    Handle<Renderable> renderable;

    static Handle<SceneNode> create(Handle<Renderable> renderable, Transform& transform);
    static void destroy(Handle<SceneNode> handle);
  };
}
#endif //SMOL_MESH_NODE_H

