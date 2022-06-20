#include <smol/smol_transform.h>
#include <smol/smol_scene.h>
#include <smol/smol_systems_root.h>

namespace smol
{
  Transform::Transform(Vector3 position, Vector3 rotation, Vector3 scale, Handle<SceneNode> parent)
    : position(position), rotation(rotation), scale(scale), dirty(true), parent(parent)
  {
  }

  const Mat4& Transform::getMatrix() const
  {
    return model; 
  }

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

  Handle<SceneNode> Transform::getParent()
  {
    return parent;
  }

  const Vector3& Transform::getPosition() const { return position; }

  const Vector3& Transform::getScale() const { return scale; }

  const Vector3& Transform::getRotation() const { return rotation; }

  bool Transform::isDirty() const { return dirty; }

  bool Transform::update(HandleList<SceneNode>* nodes)
  {
    SceneNode* parentNode = nodes->lookup(parent);
    Mat4 parentMatrix = Mat4::initIdentity();

    if(parentNode && parentNode->type != SceneNode::ROOT) // Ignores ROOT node transform and assume it's Identity
    {
      if (parentNode->transform.update(nodes))
      {
        //TODO(marcio): This is bad. It forces the transform to update every frame even if it didin't chage. Fix it!
        parentNode->transform.dirty = true;     //we keep it dirty so other children nodes can update
      }

      dirty = true;
      parentMatrix = parentNode->transform.model;
    }

    if(!dirty)
      return false;

    Mat4 scaleMatrix = Mat4::initScale(scale.x, scale.y, scale.z);
    Mat4 rotationMatrix = Mat4::initRotation(rotation.x, rotation.y, rotation.z);
    Mat4 translationMatrix = Mat4::initTranslation(position.x, position.y, position.z);

    Mat4 transformed = Mat4::mul(rotationMatrix, scaleMatrix);
    transformed = Mat4::mul(translationMatrix, transformed);
    model = Mat4::mul(parentMatrix, transformed);

    dirty = false;
    return true;
  }
}
