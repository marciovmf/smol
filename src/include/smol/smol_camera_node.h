#ifndef SMOL_CAMERA_NODE_H
#define SMOL_CAMERA_NODE_H
#include <smol/smol_camera.h>
#include <smol/smol_handle_list.h>

namespace smol
{
  struct SceneNode;
  class Transform;

  struct SMOL_ENGINE_API CameraNode final : public Camera
  {
    static Handle<SceneNode> createPerspective(float fov, float zNear, float zFar, Transform& transform);
    static Handle<SceneNode> createOrthographic(float size, float zNear, float zFar, Transform& transform);
    static void destroy(Handle<SceneNode> handle);

    private:
    CameraNode();
  };
}
#endif //SMOL_CAMERA_NODE_H

