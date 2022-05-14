#include <smol/smol_transform.h>
namespace smol
{
  Transform::Transform():
    position({.0f, .0f, .0f}), rotation(.0f, .0f, .0f),
    scale({1.0f, 1.0f, 1.0f}), angle(0.0f), dirty(false)
    { }

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

  void Transform::setScale(float x, float y, float z)
  { 
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;
  }

  void Transform::setRotation(float x, float y, float z) 
  {
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
    dirty = true;
  };

  const Vector3& Transform::getPosition() const { return position; }

  const Vector3& Transform::getScale() const { return scale; }

  const Vector3& Transform::getRotation() const { return rotation; }

  bool Transform::isDirty() const { return dirty; }

  bool Transform::update()
  {
    if(!dirty)
      return false;

    Mat4 scaleMatrix = Mat4::initScale(scale.x, scale.y, scale.z);
    Mat4 rotationMatrix = Mat4::initRotation(rotation.x, rotation.y, rotation.z);
    Mat4 translationMatrix = Mat4::initTranslation(position.x, position.y, position.z);
    
    Mat4 transformed = Mat4::mul(rotationMatrix, scaleMatrix);
    transformed = Mat4::mul(translationMatrix, transformed);

    model = transformed;
    dirty = false;
    return true;
  }
}
