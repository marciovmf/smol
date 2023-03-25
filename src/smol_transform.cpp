#include <smol/smol_transform.h>
#include <smol/smol_scene_nodes.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_scene.h>

namespace smol
{
  Transform::Transform(Vector3 position, Vector3 rotation, Vector3 scale, Handle<SceneNode> parent)
    : position(position), rotation(rotation), scale(scale), dirty(true), parent(parent)
  {
  }

  inline const Mat4& Transform::getMatrix() const { return model; }

  void Transform::setPosition(float x, float y, float z) 
  { 
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
  }

  void Transform::setPosition(const Vector3& position) 
  {
    this->position = position;
    dirty = true;
  }

  void Transform::setScale(float x, float y, float z)
  { 
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;
  }

  void Transform::setScale(const Vector3& scale) 
  {
    this->scale = scale;
    dirty = true;
  }

  void Transform::setRotation(float x, float y, float z) 
  {
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
    dirty = true;
  };

  void Transform::setRotation(const Vector3& rotation) 
  {
    this->rotation = rotation;
    dirty = true;
  }

  void Transform::setParent(Handle<SceneNode> parent)
  {
    this->parent = parent;
    dirty = true;
  }

  inline const Vector3& Transform::getPosition() const { return position; }

  inline const Vector3& Transform::getScale() const { return scale; }

  inline const Vector3& Transform::getRotation() const { return rotation; }

  inline Handle<SceneNode> Transform::getParent() const { return parent; }

  bool Transform::isDirty(const Scene& scene) const
  {
    // the Dirty flag is meant to allow us to skip matrix calculations
    // if neither the node or it's parents have changed
    SceneNode& parentNode = scene.getNode(parent);
    if (parentNode.isValid() && !parentNode.typeIs(SceneNode::ROOT))
    {
      return parentNode.transform.isDirty(scene) || dirty;
    }
    return dirty;
  }

  void Transform::update(const Scene& scene)
  {
    SceneNode& parentNode = scene.getNode(parent);
    Mat4 parentMatrix;

    // Do nothing if nothing changed
    if (isDirty(scene))
    {
      if (parentNode.isValid() && !parentNode.typeIs(SceneNode::ROOT))
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
    }
  }
}
