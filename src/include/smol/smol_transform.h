#ifndef SMOL_TRANSFORM_H
#define SMOL_TRANSFORM_H

#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_resource_list.h>

#define DEFAULT_PARENT_NODE (Handle<SceneNode>{(int) 0, (int) 0})

namespace smol
{
  struct SceneNode;

  class SMOL_ENGINE_API Transform
  {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    bool dirty;
    Handle<SceneNode> parent;
    Mat4 model;

    public:
    Transform(Vector3 position = Vector3{0.0f, 0.0f, 0.0f}, 
        Vector3 rotation = Vector3{0.0f, 0.0f, 0.0f},
        Vector3 scale = Vector3{1.0f, 1.0f, 1.0f}, 
        Handle<SceneNode> parent = DEFAULT_PARENT_NODE);

    bool update(ResourceList<SceneNode>* nodes);
    const Mat4& getMatrix() const;
    void setPosition(float x, float y, float z);
    void setPosition(const Vector3& position);
    void setScale(float x, float y, float z);
    void setScale(Vector3& scale);
    void setRotation(float x, float y, float z);
    void setRotation(Vector3& rotation);

    void setParent(Handle<SceneNode> parent);
    Handle<SceneNode> getParent();

    const Vector3& getPosition() const;
    const Vector3& getScale() const;
    const Vector3& getRotation() const;
    bool isDirty() const;
  };
}
#endif  // SMOL_TRANSFORM_H
