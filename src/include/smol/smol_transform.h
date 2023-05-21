#ifndef SMOL_TRANSFORM_H
#define SMOL_TRANSFORM_H

#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_handle_list.h>

#define DEFAULT_PARENT_NODE (Handle<SceneNode>{(int) 0, (int) 0})

namespace smol
{
  struct SceneNode;
  struct Scene;

  class SMOL_ENGINE_API Transform
  {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    bool dirty;
    Handle<SceneNode> parent;
    Mat4 model;

    public:

    Transform(
        Vector3 position  = Vector3{0.0f, 0.0f, 0.0f}, 
        Vector3 rotation  = Vector3{0.0f, 0.0f, 0.0f},
        Vector3 scale     = Vector3{1.0f, 1.0f, 1.0f}, 
        Handle<SceneNode> parent = DEFAULT_PARENT_NODE);

    bool update(const Scene& scene);

    const Mat4& getMatrix() const;

    Transform& setPosition(float x, float y, float z);

    Transform& setPosition(const Vector3& position);
   
    Transform& setScale(float x, float y, float z);
    
    Transform& setScale(const Vector3& scale);

    Transform& setRotation(float x, float y, float z);

    Transform& setRotation(const Vector3& rotation);

    Transform& setParent(Handle<SceneNode> parent);

    const Vector3& getPosition() const;

    const Vector3& getScale() const;

    const Vector3& getRotation() const;

    Handle<SceneNode> getParent() const;

    void setDirty(bool value);

    // returns true if the node or it's parents have changed this frame.
    bool isDirty(const Scene& scene) const;

  };
}
#endif  // SMOL_TRANSFORM_H
