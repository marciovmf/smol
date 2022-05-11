#ifndef SMOL_TRANSFORM_H
#define SMOL_TRANSFORM_H

#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>

namespace smol
{
  class SMOL_ENGINE_API Transform
  {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    float angle;
    bool dirty;
    Mat4 model;

    public:
    Transform::Transform();
    bool update();
    const Mat4& getMatrix() const;
    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setRotation(float x, float y, float z, float angle);
    const Vector3& getPosition() const;
    const Vector3& getScale() const;
    bool isDirty() const;
  };
}
#endif  // SMOL_TRANSFORM_H
