#ifndef SMOL_SCENE_NODE_COMMON_H
#define SMOL_SCENE_NODE_COMMON_H

#include <smol/smol_handle_list.h>
namespace smol
{
  struct SceneNode;

  struct NodeComponent
  {
    Handle<SceneNode> node;    // a reference to the node this component is attached to
  };
}
#endif //SMOL_SCENE_NODE_COMMON_H

