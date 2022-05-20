#ifndef SMOL_TRANSFORM_H
#define SMOL_TRANSFORM_H

#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_resource_list.h>

namespace smol
{
  struct SceneNode;

  class SMOL_ENGINE_API Transform
  {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    bool dirty;
    Mat4 model;
    Handle<SceneNode> parent;

    public:
    bool update(ResourceList<SceneNode>* nodes);
    const Mat4& getMatrix() const;
    void setPosition(float x, float y, float z);
    void setPosition(Vector3& position);
    void setScale(float x, float y, float z);
    void setScale(Vector3& scale);
    void setRotation(float x, float y, float z);
    void setRotation(Vector3& rotation);
    void setParent(Handle<SceneNode> parent);

    const Vector3& getPosition() const;
    const Vector3& getScale() const;
    const Vector3& getRotation() const;
    const Handle<SceneNode> getParent();
    bool isDirty() const;
  };
}
#endif  // SMOL_TRANSFORM_H
