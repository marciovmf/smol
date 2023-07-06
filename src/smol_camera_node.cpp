#include <smol/smol_camera_node.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_scene.h>

namespace smol
{
  CameraNode::CameraNode() : Camera() {}

  Handle<SceneNode> CameraNode::createPerspective(float fov, float zNear, float zFar, Transform& transform)
  {
    Scene& scene = SceneManager::get().getCurrentScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::CAMERA, transform);
    handle->camera = CameraNode();
    handle->camera.setPerspective(fov, zNear, zFar);
    return handle;
  }

  Handle<SceneNode> CameraNode::createOrthographic(float size, float zNear, float zFar, Transform& transform)
  {
    Scene& scene = SceneManager::get().getCurrentScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::CAMERA, transform);
    handle->camera = CameraNode();
    handle->camera.setOrthographic(size, zNear, zFar);
    return handle;
  }

  void CameraNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::CAMERA), "Handle passed to CameraNode::destroy() is not of type Camera");
    SceneManager::get().getCurrentScene().destroyNode(handle);
  }
}

