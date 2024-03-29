#include <smol/smol_transform.h>
#include <smol/smol_scene_node.h>
#include <smol/smol_scene.h>

namespace smol
{

  Transform::Transform(Handle<SceneNode> parent)
    : position(Vector3(0.0f)), rotation(Vector3(0.0f)), scale(Vector3(1.0f)), dirty(true), parent(parent)
  {}

  Transform::Transform(Vector3 position, Vector3 rotation, Vector3 scale, Handle<SceneNode> parent)
    : position(position), rotation(rotation), scale(scale), dirty(true), parent(parent)
  {
  }

  inline const Mat4& Transform::getMatrix() const { return model; }

  Transform& Transform::setPosition(float x, float y, float z) 
  { 
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
    return *this;
  }

  Transform& Transform::setPosition(const Vector3& position) 
  {
    this->position = position;
    dirty = true;
    return *this;
  }

  Transform& Transform::setScale(float x, float y, float z)
  { 
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;
    return *this;
  }

  Transform& Transform::setScale(const Vector3& scale) 
  {
    this->scale = scale;
    dirty = true;
    return *this;
  }

  Transform& Transform::setRotation(float x, float y, float z) 
  {
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
    dirty = true;
    return *this;
  };

  Transform& Transform::setRotation(const Vector3& rotation) 
  {
    this->rotation = rotation;
    dirty = true;
    return *this;
  }

  Transform& Transform::setParent(Handle<SceneNode> parent)
  {
    this->parent = parent;
    dirty = true;
    return *this;
  }

  Transform& Transform::unparent()
  {
    this->parent = INVALID_HANDLE(SceneNode);
    dirty = true;
    return *this;
  }

  inline const Vector3& Transform::getPosition() const { return position; }

  inline const Vector3& Transform::getScale() const { return scale; }

  inline const Vector3& Transform::getRotation() const { return rotation; }

  inline Handle<SceneNode> Transform::getParent() const { return parent; }

  bool Transform::isDirty(const Scene& scene) const
  {
    if (parent != INVALID_HANDLE(SceneNode) && parent->isValid())
    {
      return dirty || parent->transform.isDirty(scene);
    }
    return dirty;
  }


  void Transform::setDirty(bool value)
  {
    dirty = value;
  }

  bool Transform::update(const Scene& scene)
  {
    SceneNode& parentNode = *(parent.operator->());
    Mat4 parentMatrix;

    // Do nothing if nothing changed
    if (isDirty(scene))
    {
      if (parent != INVALID_HANDLE(SceneNode) && parent->isValid())
      {
        parentNode.transform.update(scene);
        parentMatrix = parentNode.transform.model;
      }
      else
      {
        parentMatrix = Mat4::initIdentity();
      }

      Mat4 scaleMatrix = Mat4::initScale(scale.x, scale.y, scale.z);
      Mat4 rotationMatrix = Mat4::initRotation(rotation.x, rotation.y, rotation.z);
      Mat4 translationMatrix = Mat4::initTranslation(position.x, position.y, position.z);

      Mat4 transformed = Mat4::mul(rotationMatrix, scaleMatrix);
      transformed = Mat4::mul(translationMatrix, transformed);
      model = Mat4::mul(parentMatrix, transformed);
      return true; // changed this frame
    }

    return false;
  }
}
